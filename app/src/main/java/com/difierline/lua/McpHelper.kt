package com.difierline.lua

import android.util.Log
import io.modelcontextprotocol.kotlin.sdk.client.Client
import io.modelcontextprotocol.kotlin.sdk.types.Implementation
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

/**
 * MCP 辅助类，用于处理与官方 MCP SDK 的交互
 * 提供 Java 可调用的接口
 */
class McpHelper {
    /**
     * 创建 MCP 客户端
     * @param name 客户端名称
     * @param version 客户端版本
     * @return MCP 客户端
     */
    fun createClient(name: String, version: String): Client {
        return Client(
            clientInfo = Implementation(
                name = name,
                version = version
            )
        )
    }
}
