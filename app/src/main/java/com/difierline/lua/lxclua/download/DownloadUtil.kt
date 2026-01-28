// DownloadUtil.kt
package com.difierline.lua.lxclua.download

import kotlinx.coroutines.*
import okhttp3.OkHttpClient
import okhttp3.ConnectionPool
import okhttp3.Request
import okhttp3.Response
import java.io.File
import java.io.RandomAccessFile
import java.io.BufferedOutputStream
import java.io.FileOutputStream
import java.net.URL
import java.net.InetAddress
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicLong
import kotlin.math.max
import kotlin.math.min

class DownloadUtil private constructor(private val config: DownloadConfig) {
    
    private val client = OkHttpClient.Builder()
        .connectTimeout(config.connectTimeout.toLong(), TimeUnit.MILLISECONDS)
        .readTimeout(config.readTimeout.toLong(), TimeUnit.MILLISECONDS)
        .connectionPool(ConnectionPool(20, 5, TimeUnit.MINUTES)) // 增加连接池
        .retryOnConnectionFailure(true)
        .build()
    
    private var downloadJob: Job? = null
    private var isPaused = false
    private var isCancelled = false
    private var totalSize = 0L
    private var downloadedSize = AtomicLong(0)
    private var lastUpdateTime = System.currentTimeMillis()
    private var lastDownloadedSize = 0L
    private var currentSpeed = 0f
    private var networkType = "wifi" // 默认WiFi
    
    private val listeners = mutableListOf<DownloadListener>()
    private val chunkProgress = ConcurrentHashMap<Int, Long>()
    
    // 保存下载参数用于恢复
    private var downloadParams: DownloadParams? = null
    
    data class DownloadParams(
        val url: String,
        val savePath: String,
        val fileName: String?
    )
    
    companion object {
        @Volatile
        private var instance: DownloadUtil? = null
        
        fun getInstance(config: DownloadConfig = DownloadConfig()): DownloadUtil {
            return instance ?: synchronized(this) {
                instance ?: DownloadUtil(config).also { instance = it }
            }
        }
    }
    
    fun setNetworkType(type: String) {
        networkType = type
    }
    
    fun addListener(listener: DownloadListener) {
        listeners.add(listener)
    }
    
    fun removeListener(listener: DownloadListener) {
        listeners.remove(listener)
    }
    
    fun download(
        url: String,
        savePath: String,
        fileName: String? = null
    ) {
        if (downloadJob?.isActive == true) {
            notifyError("已有下载任务正在进行")
            return
        }
        
        // 保存下载参数，用于恢复
        downloadParams = DownloadParams(url, savePath, fileName)
        
        downloadJob = CoroutineScope(Dispatchers.IO).launch {
            try {
                // 预连接优化
                preConnect(url)
                startDownload(url, savePath, fileName)
            } catch (e: Exception) {
                // 检查是否是暂停导致的取消
                if (e is CancellationException && isPaused) {
                    // 暂停导致的取消，不发送错误通知
                    return@launch
                }
                notifyError("下载失败: ${e.message}")
            }
        }
    }
    
    private suspend fun preConnect(url: String) {
        withContext(Dispatchers.IO) {
            try {
                // 预解析DNS
                val host = URL(url).host
                InetAddress.getAllByName(host)
                
                // 预建立连接
                val request = Request.Builder()
                    .url(url)
                    .head()
                    .build()
                
                client.newCall(request).execute().close()
            } catch (e: Exception) {
                // 预连接失败不影响主下载流程
            }
        }
    }
    
    private suspend fun startDownload(url: String, savePath: String, fileName: String?) {
        isPaused = false
        isCancelled = false
        chunkProgress.clear()
        
        // 获取文件信息
        val fileInfo = getFileInfo(url)
        totalSize = fileInfo.first
        val actualFileName = fileName ?: fileInfo.second
        
        // 动态调整线程数
        val actualThreadCount = AdaptiveDownloadStrategy.calculateOptimalThreadCount(totalSize, networkType)
        
        val saveDir = File(savePath)
        if (!saveDir.exists()) {
            saveDir.mkdirs()
        }
        
        val tempFile = File(saveDir, "$actualFileName${config.tempFileExtension}")
        val finalFile = File(saveDir, actualFileName)
        
        // 初始化临时文件
        if (!tempFile.exists()) {
            tempFile.createNewFile()
            RandomAccessFile(tempFile, "rw").use { raf ->
                raf.setLength(totalSize)
            }
        }
        
        // 计算分块（使用动态线程数）
        val chunks = calculateChunks(totalSize, actualThreadCount)
        
        // 恢复已下载的进度
        val resumeInfo = loadDownloadProgress(tempFile, chunks)
        downloadedSize.set(resumeInfo.first)
        
        notifyStart(totalSize, actualFileName)
        
        // 使用 coroutineScope 来管理并发任务
        try {
            coroutineScope {
                // 启动下载任务
                val downloadJobs = chunks.map { chunk ->
                    async {
                        downloadChunkWithBufferedIO(url, tempFile, chunk, resumeInfo.second[chunk.index] ?: 0L)
                    }
                }
                
                // 启动进度更新
                val progressJob = launch {
                    updateProgress()
                }
                
                // 等待所有下载完成
                downloadJobs.awaitAll()
                progressJob.cancel()
                
                if (isCancelled) {
                    tempFile.delete()
                    downloadParams = null
                    return@coroutineScope
                }
                
                if (!isPaused) {
                    // 重命名文件
                    tempFile.renameTo(finalFile)
                    cleanupTempFiles(tempFile, chunks)
                    downloadParams = null // 清除下载参数
                    notifyComplete(finalFile)
                }
            }
        } catch (e: CancellationException) {
            // 任务被取消（可能是暂停）
            if (isPaused) {
                // 暂停导致的取消，不发送错误通知
                return
            }
            // 其他原因导致的取消，重新抛出
            throw e
        }
    }
    
    private suspend fun getFileInfo(url: String): Pair<Long, String> {
        return withContext(Dispatchers.IO) {
            val request = Request.Builder()
                .url(url)
                .head()
                .build()
            
            client.newCall(request).execute().use { response ->
                if (!response.isSuccessful) {
                    throw Exception("无法获取文件信息: ${response.code}")
                }
                
                val contentLength = response.header("Content-Length")?.toLong() ?: 0L
                val fileName = getFileNameFromUrl(url, response)
                
                Pair(contentLength, fileName)
            }
        }
    }
    
    private fun getFileNameFromUrl(url: String, response: Response): String {
        // 从Content-Disposition头获取文件名
        val contentDisposition = response.header("Content-Disposition")
        if (contentDisposition != null) {
            val regex = "filename=\"?([^\"]+)\"?".toRegex()
            val match = regex.find(contentDisposition)
            if (match != null) {
                return match.groupValues[1]
            }
        }
        
        // 从URL获取文件名
        return url.substringAfterLast('/').takeIf { it.isNotBlank() } ?: "download_file"
    }
    
    private fun calculateChunks(totalSize: Long, threadCount: Int): List<ChunkInfo> {
        val chunks = mutableListOf<ChunkInfo>()
        
        // 使用自适应分块策略
        val optimalChunkSize = AdaptiveDownloadStrategy.calculateOptimalChunkSize(totalSize, threadCount, config)
        val actualThreadCount = min(threadCount, (totalSize / optimalChunkSize).toInt().coerceAtLeast(1))
        
        for (i in 0 until actualThreadCount) {
            val start = i * optimalChunkSize
            val end = if (i == actualThreadCount - 1) {
                totalSize - 1
            } else {
                (i + 1) * optimalChunkSize - 1
            }
            chunks.add(ChunkInfo(i, start, end))
        }
        
        return chunks
    }
    
    private fun loadDownloadProgress(tempFile: File, chunks: List<ChunkInfo>): Pair<Long, Map<Int, Long>> {
        if (!tempFile.exists()) return Pair(0L, emptyMap())
        
        val chunkProgressMap = mutableMapOf<Int, Long>()
        var totalDownloaded = 0L
        
        chunks.forEach { chunk ->
            val chunkFile = File("${tempFile.absolutePath}.chunk${chunk.index}")
            if (chunkFile.exists()) {
                val downloaded = chunkFile.readText().toLongOrNull() ?: 0L
                chunkProgressMap[chunk.index] = downloaded
                totalDownloaded += downloaded
            }
        }
        
        return Pair(totalDownloaded, chunkProgressMap)
    }
    
    private fun cleanupTempFiles(tempFile: File, chunks: List<ChunkInfo>) {
        try {
            // 删除主临时文件
            if (tempFile.exists()) {
                tempFile.delete()
            }
            
            // 删除所有分块进度文件
            chunks.forEach { chunk ->
                val chunkFile = File("${tempFile.absolutePath}.chunk${chunk.index}")
                if (chunkFile.exists()) {
                    chunkFile.delete()
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
    
    private suspend fun downloadChunkWithBufferedIO(
        url: String, 
        file: File, 
        chunk: ChunkInfo, 
        resumeFrom: Long
    ) {
        var currentStart = chunk.start + resumeFrom
        var retryCount = 0
        var currentBufferSize = config.bufferSize
        
        while (currentStart <= chunk.end && !isPaused && !isCancelled) {
            try {
                withContext(Dispatchers.IO) {
                    val request = Request.Builder()
                        .url(url)
                        .header("Range", "bytes=$currentStart-${chunk.end}")
                        .build()
                    
                    client.newCall(request).execute().use { response ->
                        if (!response.isSuccessful) {
                            throw Exception("下载失败: ${response.code}")
                        }
                        
                        response.body?.byteStream()?.use { inputStream ->
                            RandomAccessFile(file, "rw").use { raf ->
                                raf.seek(currentStart)
                                
                                // 修复：使用 FileOutputStream 包装 RandomAccessFile
                                val outputStream = FileOutputStream(raf.fd)
                                val bufferedOutputStream = BufferedOutputStream(outputStream, currentBufferSize)
                                
                                val buffer = ByteArray(currentBufferSize)
                                var bytesRead: Int
                                var chunkDownloaded = resumeFrom
                                var chunkStartTime = System.currentTimeMillis()
                                var lastSpeedCheckTime = chunkStartTime
                                var lastSpeedCheckBytes = 0L
                                
                                while (inputStream.read(buffer).also { bytesRead = it } != -1 
                                    && !isPaused && !isCancelled) {
                                    bufferedOutputStream.write(buffer, 0, bytesRead)
                                    chunkDownloaded += bytesRead
                                    downloadedSize.addAndGet(bytesRead.toLong())
                                    chunkProgress[chunk.index] = chunkDownloaded
                                    currentStart += bytesRead
                                    
                                    // 动态调整缓冲区大小
                                    val currentTime = System.currentTimeMillis()
                                    if (currentTime - lastSpeedCheckTime > 2000) { // 每2秒检查一次
                                        val timeDiff = (currentTime - lastSpeedCheckTime) / 1000f
                                        val bytesDiff = chunkDownloaded - lastSpeedCheckBytes
                                        val chunkSpeed = bytesDiff / timeDiff
                                        
                                        // 根据速度调整缓冲区
                                        currentBufferSize = AdaptiveDownloadStrategy.calculateOptimalBufferSize(chunkSpeed)
                                        
                                        lastSpeedCheckTime = currentTime
                                        lastSpeedCheckBytes = chunkDownloaded
                                    }
                                    
                                    // 保存分块进度（降低频率以提高性能）
                                    if (chunkDownloaded % (2 * 1024 * 1024) == 0L) { // 每2MB保存一次
                                        saveChunkProgress(file, chunk.index, chunkDownloaded)
                                    }
                                }
                                
                                bufferedOutputStream.flush()
                                bufferedOutputStream.close()
                                
                                // 下载完成后保存最终进度
                                saveChunkProgress(file, chunk.index, chunkDownloaded)
                            }
                        }
                    }
                }
                break // 下载完成，退出重试循环
            } catch (e: Exception) {
                retryCount++
                if (retryCount > config.retryCount) {
                    notifyError("分块 ${chunk.index} 下载失败: ${e.message}")
                    break
                }
                delay(config.retryDelay * retryCount) // 递增重试延迟
            }
        }
    }
    
    private fun saveChunkProgress(file: File, chunkIndex: Int, downloaded: Long) {
        try {
            val chunkFile = File("${file.absolutePath}.chunk$chunkIndex")
            chunkFile.writeText(downloaded.toString())
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
    
    private suspend fun updateProgress() {
        while (!isPaused && !isCancelled) {
            try {
                delay(500) // 每500ms更新一次，更频繁的更新
            
                val currentTime = System.currentTimeMillis()
                val timeDiff = (currentTime - lastUpdateTime) / 1000f
                val sizeDiff = downloadedSize.get() - lastDownloadedSize
                
                currentSpeed = sizeDiff / timeDiff
                
                val progress = if (totalSize > 0) {
                    (downloadedSize.get().toFloat() / totalSize.toFloat()) * 100
                } else {
                    0f
                }
                
                notifyProgress(downloadedSize.get(), totalSize, currentSpeed, progress)
                
                lastUpdateTime = currentTime
                lastDownloadedSize = downloadedSize.get()
            } catch (e: CancellationException) {
                // 协程被取消时退出循环
                break
            }
        }
    }
    
    fun pause() {
        isPaused = true
        downloadJob?.cancel() // 取消当前任务
        downloadJob = null
        notifyPause(downloadedSize.get(), totalSize)
    }
    
    fun resume() {
        if (isPaused && downloadParams != null) {
            isPaused = false
            isCancelled = false
            
            // 重新开始下载
            downloadParams?.let { params ->
                downloadJob = CoroutineScope(Dispatchers.IO).launch {
                    try {
                        startDownload(params.url, params.savePath, params.fileName)
                    } catch (e: Exception) {
                        notifyError("恢复下载失败: ${e.message}")
                    }
                }
            }
            notifyResume(downloadedSize.get(), totalSize)
        } else {
            notifyError("无法恢复：没有可恢复的下载任务")
        }
    }
    
    fun cancel() {
        isCancelled = true
        isPaused = false
        downloadJob?.cancel()
        downloadJob = null
        downloadParams = null // 清除下载参数
        notifyError("下载已取消")
    }
    
    // 通知方法
    private fun notifyStart(totalSize: Long, fileName: String) {
        listeners.forEach { it.onStart(totalSize, fileName) }
    }
    
    private fun notifyProgress(downloaded: Long, total: Long, speed: Float, progress: Float) {
        listeners.forEach { it.onProgress(downloaded, total, speed, progress) }
    }
    
    private fun notifyComplete(file: File) {
        listeners.forEach { it.onComplete(file) }
    }
    
    private fun notifyError(error: String) {
        listeners.forEach { it.onError(error) }
    }
    
    private fun notifyPause(downloaded: Long, total: Long) {
        listeners.forEach { it.onPause(downloaded, total) }
    }
    
    private fun notifyResume(downloaded: Long, total: Long) {
        listeners.forEach { it.onResume(downloaded, total) }
    }
    
    fun getCurrentSpeed(): Float = currentSpeed
    fun getProgress(): Float = if (totalSize > 0) {
        (downloadedSize.get().toFloat() / totalSize.toFloat()) * 100
    } else {
        0f
    }
    fun isDownloading(): Boolean = downloadJob?.isActive == true
    fun isPaused(): Boolean = isPaused
    fun hasDownloadTask(): Boolean = downloadParams != null
}