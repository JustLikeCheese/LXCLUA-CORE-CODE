// AdaptiveDownloadStrategy.kt
package com.difierline.lua.lxclua.download

class AdaptiveDownloadStrategy {
    companion object {
        fun calculateOptimalThreadCount(fileSize: Long, networkType: String = "wifi"): Int {
            return when {
                fileSize > 100 * 1024 * 1024 -> when (networkType) {
                    "wifi" -> 12
                    "5g" -> 8
                    "4g" -> 6
                    else -> 4
                }
                fileSize > 10 * 1024 * 1024 -> 8
                fileSize > 1 * 1024 * 1024 -> 6
                else -> 4
            }
        }
        
        fun calculateOptimalBufferSize(networkSpeed: Float): Int {
            return when {
                networkSpeed > 10 * 1024 * 1024 -> 65536  // 64KB for high speed
                networkSpeed > 1 * 1024 * 1024 -> 32768   // 32KB for medium speed
                else -> 16384                             // 16KB for low speed
            }
        }
        
        fun calculateOptimalChunkSize(totalSize: Long, threadCount: Int, config: DownloadConfig): Long {
            val baseChunkSize = totalSize / threadCount
            return baseChunkSize.coerceIn(config.minChunkSize, config.maxChunkSize)
        }
    }
}