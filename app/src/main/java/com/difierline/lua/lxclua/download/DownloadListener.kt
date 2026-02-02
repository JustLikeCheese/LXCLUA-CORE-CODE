// DownloadListener.kt
package com.difierline.lua.lxclua.download

import java.io.File

interface DownloadListener {
    fun onStart(totalSize: Long, fileName: String)
    fun onProgress(downloaded: Long, total: Long, speed: Float, progress: Float)
    fun onComplete(file: File)
    fun onError(error: String)
    fun onPause(downloaded: Long, total: Long)
    fun onResume(downloaded: Long, total: Long)
}