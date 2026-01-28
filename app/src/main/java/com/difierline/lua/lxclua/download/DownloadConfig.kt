// DownloadConfig.kt
package com.difierline.lua.lxclua.download

data class DownloadConfig(
    val threadCount: Int = 8,                    // 下载线程数，增加到8个
    val bufferSize: Int = 32768,                 // 缓冲区大小，增加到32KB
    val connectTimeout: Int = 15000,             // 连接超时时间，增加到15秒
    val readTimeout: Int = 60000,                // 读取超时时间，增加到60秒
    val retryCount: Int = 5,                     // 重试次数，增加到5次
    val retryDelay: Long = 500,                  // 重试延迟(ms)，减少到500ms
    val tempFileExtension: String = ".tmp",      // 临时文件扩展名
    val minChunkSize: Long = 2 * 1024 * 1024,   // 最小分块大小2MB
    val maxChunkSize: Long = 10 * 1024 * 1024   // 最大分块大小10MB
)