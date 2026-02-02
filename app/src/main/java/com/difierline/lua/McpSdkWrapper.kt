package com.difierline.lua

import io.modelcontextprotocol.kotlin.sdk.client.Client
import io.modelcontextprotocol.kotlin.sdk.shared.AbstractTransport
import io.modelcontextprotocol.kotlin.sdk.types.CallToolRequest
import io.modelcontextprotocol.kotlin.sdk.types.CallToolRequestParams
import io.modelcontextprotocol.kotlin.sdk.types.Implementation
import io.modelcontextprotocol.kotlin.sdk.types.Tool
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.future.future
import kotlinx.coroutines.withContext
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonPrimitive
import java.util.concurrent.CompletableFuture

/**
 * MCP SDK 包装类，用于简化 Java 调用官方 SDK
 */
object McpSdkWrapper {
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
    
    /**
     * 连接 MCP 客户端到服务器
     * @param client MCP 客户端
     * @param transport 传输层实现
     * @return CompletableFuture，连接完成后触发
     */
    fun connect(client: Client, transport: AbstractTransport): CompletableFuture<Void> {
        return GlobalScope.future(Dispatchers.IO) {
            withContext(Dispatchers.IO) {
                client.connect(transport)
            }
        }.thenApply { null }
    }
    
    /**
     * 获取服务器上的工具列表
     * @param client MCP 客户端
     * @return CompletableFuture，包含工具列表
     */
    fun listTools(client: Client): CompletableFuture<List<Tool>> {
        return GlobalScope.future(Dispatchers.IO) {
            withContext(Dispatchers.IO) {
                client.listTools()?.tools ?: emptyList()
            }
        }
    }
    
    /**
     * 调用 MCP 工具
     * @param client MCP 客户端
     * @param toolName 工具名称
     * @param arguments 工具参数
     * @return CompletableFuture，包含工具调用结果
     */
    fun callTool(client: Client, toolName: String, arguments: Map<String, Any>): CompletableFuture<Any> {
        return GlobalScope.future(Dispatchers.IO) {
            withContext(Dispatchers.IO) {
                // 转换 Map<String, Any> 为 JsonObject
                val jsonArguments = arguments.entries.associate {
                    it.key to when (val value = it.value) {
                        is String -> JsonPrimitive(value)
                        is Number -> JsonPrimitive(value)
                        is Boolean -> JsonPrimitive(value)
                        else -> JsonPrimitive(value.toString())
                    }
                }.let { JsonObject(it) }
                
                val result = client.callTool(
                    request = CallToolRequest(
                        params = CallToolRequestParams(
                            name = toolName,
                            arguments = jsonArguments
                        )
                    )
                )
                result.content
            }
        }
    }
    
    /**
     * 关闭 MCP 客户端连接
     * @param client MCP 客户端
     * @return CompletableFuture，关闭完成后触发
     */
    fun close(client: Client): CompletableFuture<Void> {
        return GlobalScope.future(Dispatchers.IO) {
            withContext(Dispatchers.IO) {
                client.close()
            }
        }.thenApply { null }
    }
}
