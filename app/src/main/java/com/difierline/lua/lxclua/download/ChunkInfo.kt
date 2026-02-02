// ChunkInfo.kt
package com.difierline.lua.lxclua.download

data class ChunkInfo(
    val index: Int,              // 分块索引
    val start: Long,             // 起始位置
    val end: Long,               // 结束位置
    val downloaded: Long = 0,    // 已下载字节数
    var isCompleted: Boolean = false
)