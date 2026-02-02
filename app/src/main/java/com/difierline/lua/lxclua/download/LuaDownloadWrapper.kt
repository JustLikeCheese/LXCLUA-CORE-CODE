// LuaDownloadWrapper.kt
package com.difierline.lua.lxclua.download

import android.content.Context
import android.net.ConnectivityManager
import android.os.Build
import android.os.Handler
import android.os.Looper
import java.io.File
import com.luajava.*
import com.difierline.lua.LuaContext

class LuaDownloadWrapper(private val context: Context) {
    
    private val downloadUtil = DownloadUtil.getInstance()
    private val mainHandler = Handler(Looper.getMainLooper())
    
    // 存储 Lua 回调函数
    private val luaCallbacks = mutableMapOf<String, LuaObject?>()
    
    init {
        // 初始化时检测网络类型
        detectNetworkType()
    }
    
    fun download(url: String, savePath: String, fileName: String? = null, callbacks: Map<String, LuaObject>? = null) {
        // 检测当前网络类型
        detectNetworkType()
        
        // 如果有传入回调字典，则设置回调
        callbacks?.forEach { (event, callback) ->
            luaCallbacks[event] = callback
        }
        
        downloadUtil.addListener(object : DownloadListener {
            override fun onStart(totalSize: Long, fileName: String) {
                callLuaFunction("onStart", totalSize.toDouble(), fileName)
            }
            
            override fun onProgress(downloaded: Long, total: Long, speed: Float, progress: Float) {
                callLuaFunction("onProgress", 
                    downloaded.toDouble(), 
                    total.toDouble(), 
                    speed.toDouble(), 
                    progress.toDouble())
            }
            
            override fun onComplete(file: File) {
                // 下载完成后清理临时文件
                cleanupTempFiles(file.parent, fileName ?: getFileNameFromUrl(url))
                callLuaFunction("onComplete", file.absolutePath)
            }
            
            override fun onError(error: String) {
                callLuaFunction("onError", error)
            }
            
            override fun onPause(downloaded: Long, total: Long) {
                callLuaFunction("onPause", downloaded.toDouble(), total.toDouble())
            }
            
            override fun onResume(downloaded: Long, total: Long) {
                callLuaFunction("onResume", downloaded.toDouble(), total.toDouble())
            }
        })
        
        downloadUtil.download(url, savePath, fileName)
    }
    
    // 新的设置回调方法，接受字典参数
    fun setCallbacks(callbacks: Map<String, LuaObject>) {
        callbacks.forEach { (event, callback) ->
            luaCallbacks[event] = callback
        }
    }
    
    // 兼容旧版本的单回调设置方法
    fun setLuaCallback(event: String, callback: LuaObject?) {
        luaCallbacks[event] = callback
    }
    
    // 设置网络类型
    fun setNetworkType(type: String) {
        downloadUtil.setNetworkType(type)
    }
    
    fun pause() {
        downloadUtil.pause()
    }
    
    fun resume() {
        downloadUtil.resume()
    }
    
    fun cancel() {
        downloadUtil.cancel()
    }
    
    fun getCurrentSpeed(): Float {
        return downloadUtil.getCurrentSpeed()
    }
    
    fun getProgress(): Float {
        return downloadUtil.getProgress()
    }
    
    fun isDownloading(): Boolean {
        return downloadUtil.isDownloading()
    }
    
    fun isPaused(): Boolean {
        return downloadUtil.isPaused()
    }
    
    fun hasDownloadTask(): Boolean {
        return downloadUtil.hasDownloadTask()
    }
    
    private fun detectNetworkType() {
        try {
            val connectivityManager = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
            val networkInfo = connectivityManager.activeNetworkInfo
            
            if (networkInfo != null && networkInfo.isConnected) {
                when (networkInfo.type) {
                    ConnectivityManager.TYPE_WIFI -> downloadUtil.setNetworkType("wifi")
                    ConnectivityManager.TYPE_MOBILE -> {
                        // 修复：使用兼容的方式检测网络类型
                        when (networkInfo.subtype) {
                            getNetworkTypeConstant("LTE"),
                            getNetworkTypeConstant("NR") -> downloadUtil.setNetworkType("5g")
                            getNetworkTypeConstant("HSPAP") -> downloadUtil.setNetworkType("4g")
                            getNetworkTypeConstant("HSDPA"),
                            getNetworkTypeConstant("HSUPA") -> downloadUtil.setNetworkType("3g")
                            else -> {
                                // 根据 subtype 名称判断
                                val subtypeName = networkInfo.subtypeName
                                when {
                                    subtypeName.contains("LTE", true) || subtypeName.contains("NR", true) -> 
                                        downloadUtil.setNetworkType("5g")
                                    subtypeName.contains("HSPA", true) || subtypeName.contains("HSDPA", true) || 
                                    subtypeName.contains("HSUPA", true) -> downloadUtil.setNetworkType("4g")
                                    else -> downloadUtil.setNetworkType("3g")
                                }
                            }
                        }
                    }
                    else -> downloadUtil.setNetworkType("unknown")
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
    
    private fun getNetworkTypeConstant(type: String): Int {
        return try {
            val field = ConnectivityManager::class.java.getField("NETWORK_TYPE_$type")
            field.getInt(null)
        } catch (e: Exception) {
            // 如果常量不存在，返回默认值
            when (type) {
                "LTE" -> 13
                "NR" -> 20
                "HSPAP" -> 15
                "HSDPA" -> 8
                "HSUPA" -> 9
                else -> -1
            }
        }
    }
    
    private fun callLuaFunction(event: String, vararg args: Any) {
        val callback = luaCallbacks[event]
        if (callback != null && callback.isFunction()) {
            try {
                // 在主线程中执行 Lua 回调
                mainHandler.post {
                    try {
                        callback.call(*args)
                    } catch (e: Exception) {
                        e.printStackTrace()
                    }
                }
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }
    
    private fun getFileNameFromUrl(url: String): String {
        return url.substringAfterLast('/').takeIf { it.isNotBlank() } ?: "download_file"
    }
    
    // 清理临时文件
    private fun cleanupTempFiles(directory: String, baseFileName: String) {
        try {
            val dir = File(directory)
            if (dir.exists()) {
                val files = dir.listFiles()
                files?.forEach { file ->
                    if (file.name.startsWith(baseFileName) && 
                        (file.name.endsWith(".tmp") || file.name.contains(".chunk"))) {
                        file.delete()
                    }
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
}