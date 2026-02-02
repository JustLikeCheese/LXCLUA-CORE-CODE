package com.difierline.lua.lxclua.utils.exceptions

import android.content.Context
import android.util.Log
import java.io.PrintWriter
import java.io.StringWriter
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.concurrent.ConcurrentHashMap

/**
 * 增强的错误处理器 - 提供详细的中文错误信息
 * 支持错误分类、堆栈跟踪解析、错误统计等功能
 */
class LuaErrorHandler private constructor() {

    companion object {
        private const val TAG = "LuaErrorHandler"

        @Volatile
        private var instance: LuaErrorHandler? = null

        fun getInstance(): LuaErrorHandler {
            return instance ?: synchronized(this) {
                instance ?: LuaErrorHandler().also { instance = it }
            }
        } 
    }

    /**
     * 错误类型枚举
     */
    enum class ErrorType {
        LUA_SYNTAX,           // Lua 语法错误
        LUA_RUNTIME,          // Lua 运行时错误
        LUA_MEMORY,           // Lua 内存错误
        LUA_TYPE,             // Lua 类型错误
        LUA_INDEX,            // Lua 索引错误
        LUA_CALLBACK,         // Lua 回调错误
        VIEW_CREATE,          // 视图创建错误
        VIEW_ATTRIBUTE,       // 视图属性错误
        VIEW_PARSE,           // 视图解析错误
        LAYOUT_LOAD,          // 布局加载错误
        RESOURCE_NOT_FOUND,   // 资源未找到
        PERMISSION_DENIED,    // 权限拒绝
        NULL_POINTER,         // 空指针错误
        CLASS_NOT_FOUND,      // 类未找到
        METHOD_NOT_FOUND,     // 方法未找到
        INVOKE_ERROR,         // 调用错误
        JSON_PARSE,           // JSON 解析错误
        FILE_IO,              // 文件 IO 错误
        DATABASE,             // 数据库错误
        NETWORK,              // 网络错误
        UNKNOWN               // 未知错误
    }

    /**
     * 错误详情类
     */
    data class ErrorDetail(
        val type: ErrorType,
        val message: String,
        val stackTrace: String,
        val fileName: String?,
        val lineNumber: Int?,
        val methodName: String?,
        val timestamp: Long = System.currentTimeMillis(),
        val contextInfo: String? = null
    ) {
        /**
         * 获取中文错误描述
         */
        fun getChineseDescription(): String {
            val typeDesc = when (type) {
                ErrorType.LUA_SYNTAX -> "[X] 【Lua 语法错误】"
                ErrorType.LUA_RUNTIME -> "[X] 【Lua 运行时错误】"
                ErrorType.LUA_MEMORY -> "[X] 【Lua 内存错误】"
                ErrorType.LUA_TYPE -> "[X] 【Lua 类型错误】"
                ErrorType.LUA_INDEX -> "[X] 【Lua 索引错误】"
                ErrorType.LUA_CALLBACK -> "[X] 【Lua 回调错误】"
                ErrorType.VIEW_CREATE -> "[X] 【视图创建错误】"
                ErrorType.VIEW_ATTRIBUTE -> "[X] 【视图属性错误】"
                ErrorType.VIEW_PARSE -> "[X] 【视图解析错误】"
                ErrorType.LAYOUT_LOAD -> "[X] 【布局加载错误】"
                ErrorType.RESOURCE_NOT_FOUND -> "[X] 【资源未找到】"
                ErrorType.PERMISSION_DENIED -> "[X] 【权限拒绝】"
                ErrorType.NULL_POINTER -> "[X] 【空指针错误】"
                ErrorType.CLASS_NOT_FOUND -> "[X] 【类未找到】"
                ErrorType.METHOD_NOT_FOUND -> "[X] 【方法未找到】"
                ErrorType.INVOKE_ERROR -> "[X] 【调用错误】"
                ErrorType.JSON_PARSE -> "[X] 【JSON 解析错误】"
                ErrorType.FILE_IO -> "[X] 【文件 IO 错误】"
                ErrorType.DATABASE -> "[X] 【数据库错误】"
                ErrorType.NETWORK -> "[X] 【网络错误】"
                ErrorType.UNKNOWN -> "[X] 【未知错误】"
            }

            val locationInfo = buildString {
                fileName?.let { append("文件: $it") }
                lineNumber?.let { append(", 行号: $it") }
                methodName?.let { append(", 方法: $it") }
            }

            return buildString {
                appendLine(typeDesc)
                appendLine("错误信息: $message")
                if (locationInfo.isNotEmpty()) {
                    appendLine("位置信息: $locationInfo")
                }
                contextInfo?.let { appendLine("上下文: $it") }
                appendLine("发生时间: ${formatTimestamp(timestamp)}")
            }
        }

        /**
         * 获取友好的用户提示
         */
        fun getUserSuggestion(): String {
            return when (type) {
                ErrorType.LUA_SYNTAX -> "请检查 Lua 语法，如括号是否配对、关键字是否正确"
                ErrorType.LUA_RUNTIME -> "运行时发生错误，请检查变量值和函数调用是否正确"
                ErrorType.LUA_TYPE -> "类型不匹配，请检查变量类型是否正确"
                ErrorType.LUA_INDEX -> "索引无效，请检查数组或表的索引是否越界"
                ErrorType.VIEW_CREATE -> "视图创建失败，请检查类名是否正确、是否缺少必要的参数"
                ErrorType.VIEW_ATTRIBUTE -> "视图属性设置失败，请检查属性名和属性值是否正确"
                ErrorType.LAYOUT_LOAD -> "布局加载失败，请检查布局文件是否存在、格式是否正确"
                ErrorType.RESOURCE_NOT_FOUND -> "资源文件未找到，请检查文件路径是否正确"
                ErrorType.PERMISSION_DENIED -> "权限被拒绝，请检查 AndroidManifest.xml 中是否声明了相应权限"
                ErrorType.NULL_POINTER -> "对象为空，请检查对象是否已正确初始化"
                ErrorType.CLASS_NOT_FOUND -> "类未找到，请检查类名是否正确、是否已导入"
                ErrorType.METHOD_NOT_FOUND -> "方法未找到，请检查方法名是否正确、方法是否存在"
                ErrorType.JSON_PARSE -> "JSON 解析失败，请检查 JSON 格式是否正确"
                ErrorType.FILE_IO -> "文件操作失败，请检查文件路径和权限"
                ErrorType.DATABASE -> "数据库操作失败，请检查数据库是否正常、SQL 语句是否正确"
                ErrorType.NETWORK -> "网络请求失败，请检查网络连接和 URL 是否正确"
                else -> "请检查相关代码逻辑，或联系开发者获取帮助"
            }
        }

        private fun formatTimestamp(timestamp: Long): String {
            val sdf = SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.getDefault())
            return sdf.format(Date(timestamp))
        }
    }

    private val errorHistory = ConcurrentHashMap<String, MutableList<ErrorDetail>>()
    private val errorListeners = mutableListOf<ErrorListener>()

    interface ErrorListener {
        fun onError(error: ErrorDetail)
    }

    /**
     * 注册错误监听器
     */
    fun addErrorListener(listener: ErrorListener) {
        errorListeners.add(listener)
    }

    /**
     * 移除错误监听器
     */
    fun removeErrorListener(listener: ErrorListener) {
        errorListeners.remove(listener)
    }

    /**
     * 处理 Lua 错误
     */
    fun handleLuaError(errorMessage: String, stackTrace: String, context: String? = null): ErrorDetail {
        val type = classifyLuaError(errorMessage)
        val detail = parseStackTrace(stackTrace, type, errorMessage, context)
        recordError(detail)
        notifyError(detail)
        return detail
    }

    /**
     * 处理视图创建错误
     */
    fun handleViewCreateError(viewClassName: String, error: Throwable, layoutInfo: String? = null): ErrorDetail {
        val detail = ErrorDetail(
            type = ErrorType.VIEW_CREATE,
            message = "创建视图 $viewClassName 失败: ${error.message}",
            stackTrace = getStackTrace(error),
            fileName = viewClassName,
            lineNumber = null,
            methodName = "init",
            contextInfo = layoutInfo
        )
        recordError(detail)
        notifyError(detail)
        return detail
    }

    /**
     * 处理布局加载错误
     */
    fun handleLayoutLoadError(layoutPath: String, error: Throwable, details: String? = null): ErrorDetail {
        val detail = ErrorDetail(
            type = ErrorType.LAYOUT_LOAD,
            message = "加载布局 $layoutPath 失败: ${error.message}",
            stackTrace = getStackTrace(error),
            fileName = layoutPath,
            lineNumber = null,
            methodName = "loadlayout",
            contextInfo = details
        )
        recordError(detail)
        notifyError(detail)
        return detail
    }

    /**
     * 处理属性设置错误
     */
    fun handleAttributeError(
        viewClassName: String,
        attributeName: String,
        attributeValue: String,
        error: Throwable
    ): ErrorDetail {
        val detail = ErrorDetail(
            type = ErrorType.VIEW_ATTRIBUTE,
            message = "为 $viewClassName 设置属性 $attributeName=$attributeValue 失败: ${error.message}",
            stackTrace = getStackTrace(error),
            fileName = viewClassName,
            lineNumber = null,
            methodName = "set${attributeName.replaceFirstChar { it.uppercase() }}",
            contextInfo = "属性值: $attributeValue"
        )
        recordError(detail)
        notifyError(detail)
        return detail
    }

    /**
     * 处理通用异常
     */
    fun handleException(throwable: Throwable, context: String? = null): ErrorDetail {
        val type = ErrorType.UNKNOWN
        val detail = parseStackTrace(
            getStackTrace(throwable),
            type,
            throwable.message ?: "未知错误",
            context
        )
        recordError(detail)
        notifyError(detail)
        return detail
    }

    /**
     * 分类 Lua 错误类型
     */
    private fun classifyLuaError(errorMessage: String): ErrorType {
        return when {
            errorMessage.contains("语法错误") || errorMessage.contains("syntax error") ->
                ErrorType.LUA_SYNTAX
            errorMessage.contains("索引") || errorMessage.contains("attempt to index") ->
                ErrorType.LUA_INDEX
            errorMessage.contains("调用") || errorMessage.contains("attempt to call") ->
                ErrorType.LUA_CALLBACK
            errorMessage.contains("错误的参数") || errorMessage.contains("参数错误") ||
            errorMessage.contains("bad argument") || errorMessage.contains("错误的") ->
                ErrorType.LUA_TYPE
            errorMessage.contains("内存不足") || errorMessage.contains("out of memory") ->
                ErrorType.LUA_MEMORY
            errorMessage.contains("栈溢出") || errorMessage.contains("stack overflow") ->
                ErrorType.LUA_MEMORY
            errorMessage.contains("空指针") || errorMessage.contains("null pointer") ->
                ErrorType.NULL_POINTER
            errorMessage.contains("无法修改") || errorMessage.contains("cannot change") ||
            errorMessage.contains("cannot modify") ->
                ErrorType.LUA_TYPE
            errorMessage.contains("无法打开文件") || errorMessage.contains("cannot open file") ->
                ErrorType.FILE_IO
            errorMessage.contains("模块") && errorMessage.contains("未找到") ||
            errorMessage.contains("module") && errorMessage.contains("not found") ->
                ErrorType.LUA_RUNTIME
            else -> ErrorType.LUA_RUNTIME
        }
    }

    /**
     * 解析堆栈跟踪
     */
    private fun parseStackTrace(
        stackTrace: String,
        type: ErrorType,
        message: String,
        context: String?
    ): ErrorDetail {
        var fileName: String? = null
        var lineNumber: Int? = null
        var methodName: String? = null

        val lines = stackTrace.split("\n")
        for (line in lines) {
            when {
                line.contains(".lua:") -> {
                    val matcher = Regex("(.+?)\\((\\d+)\\)").find(line)
                    if (matcher != null) {
                        fileName = matcher.groupValues[1].substringAfterLast("/")
                            .substringAfterLast("\\")
                        lineNumber = matcher.groupValues[2].toIntOrNull()
                    }
                }
                line.contains("function:") -> {
                    methodName = line.substringAfterLast(":").substringBefore("@")
                }
            }
        }

        return ErrorDetail(
            type = type,
            message = message,
            stackTrace = stackTrace,
            fileName = fileName,
            lineNumber = lineNumber,
            methodName = methodName,
            contextInfo = context
        )
    }

    /**
     * 获取堆栈跟踪字符串
     */
    private fun getStackTrace(throwable: Throwable): String {
        val sw = StringWriter()
        throwable.printStackTrace(PrintWriter(sw))
        return sw.toString()
    }

    /**
     * 记录错误
     */
    private fun recordError(detail: ErrorDetail) {
        val key = detail.type.name
        val list = errorHistory.getOrPut(key) { mutableListOf() }
        synchronized(list) {
            list.add(0, detail)
            if (list.size > 100) {
                list.removeAt(list.lastIndex)
            }
        }
        Log.e(TAG, "Error recorded: ${detail.type.name} - ${detail.message}")
    }

    /**
     * 通知错误监听器
     */
    private fun notifyError(detail: ErrorDetail) {
        errorListeners.forEach { it.onError(detail) }
    }

    /**
     * 获取错误历史
     */
    fun getErrorHistory(errorType: ErrorType? = null): List<ErrorDetail> {
        return if (errorType != null) {
            errorHistory[errorType.name]?.toList() ?: emptyList()
        } else {
            errorHistory.values.flatten()
        }
    }

    /**
     * 清除错误历史
     */
    fun clearHistory() {
        errorHistory.clear()
    }

    /**
     * 获取错误统计
     */
    fun getErrorStatistics(): Map<ErrorType, Int> {
        val result = mutableMapOf<ErrorType, Int>()
        errorHistory.forEach { (key, value) ->
            try {
                val type = ErrorType.valueOf(key)
                result[type] = value.size
            } catch (e: Exception) {
                // 忽略无效的 key
            }
        }
        return result
    }

    /**
     * 格式化错误为友好字符串
     */
    fun formatErrorForDisplay(error: ErrorDetail): String {
        return buildString {
            appendLine("=================================================")
            appendLine(error.getChineseDescription())
            appendLine()
            appendLine("[Tip] 解决建议:")
            appendLine("  ${error.getUserSuggestion()}")
            appendLine()
            appendLine("[Debug] 堆栈跟踪:")
            appendLine(error.stackTrace)
            appendLine("=================================================")
        }
    }

    /**
     * 获取所有错误的汇总报告
     */
    fun getErrorReport(): String {
        val stats = getErrorStatistics()
        if (stats.isEmpty()) {
            return "暂无错误记录"
        }

        return buildString {
            appendLine("═" .repeat(60))
            appendLine("  错误统计报告")
            appendLine("═" .repeat(60))
            appendLine()

            val sortedStats = stats.entries.sortedByDescending { it.value }
            for ((type, count) in sortedStats) {
                appendLine("  ${type.name.padEnd(20)} : $count 次")
            }
            appendLine()
            appendLine("  总计: ${stats.values.sum()} 个错误")
            appendLine("═" .repeat(60))
        }
    }
}

/**
 * Lua 布局错误解析器
 */
object LuaLayoutErrorParser {

    private val errorPatterns = mapOf(
        Regex("(.+?):(\\d+): .- near (.+)") to "语法错误",
        Regex("错误的参数 #(\\d+)") to "参数错误",
        Regex("调用 '.+' 时错误的 self") to "self错误",
        Regex("错误的 (.+) #\\d+ 传递给") to "参数错误",
        Regex("栈溢出") to "内存错误",
        Regex("结果字符串过长") to "字符串过长",
        Regex("模块 '.+' 命名冲突") to "模块冲突",
        Regex("对象长度不是整数") to "类型错误",
        Regex("'__tostring' 必须返回字符串") to "类型错误",
        Regex("核心与库的数字类型不兼容") to "类型不匹配",
        Regex("版本不匹配") to "版本错误",
        Regex("无法获取 ModuleFileName") to "系统错误",
        Regex("'package\\..+' 必须是字符串") to "类型错误",
        Regex("'package\\.searchers' 必须是表") to "类型错误",
        Regex("模块 '.+' 未找到") to "模块未找到",
        Regex("'module' 不是从 Lua 函数调用的") to "调用错误",
        Regex("'popen' 不支持") to "不支持的函数",
        Regex("无法打开文件") to "文件错误",
        Regex("文件已关闭") to "文件错误",
        Regex("默认 .+ 文件已关闭") to "文件错误",
        Regex("无法修改受保护的") to "权限错误",
        Regex("读取函数必须返回字符串") to "类型错误",
        Regex("错误的参数 #1 传递给") to "参数错误",
        Regex("无法修改受保护的函数") to "权限错误",
        Regex("空指针") to "空指针错误",
        Regex("需要数字或字符串") to "类型错误",
        Regex("字符串切片过长") to "内存错误",
        Regex("无效的转义字符") to "语法错误",
        Regex("无效的转义序列") to "语法错误",
        Regex("起始位置是延续字节") to "编码错误",
        Regex("无效的捕获索引") to "模式错误",
        Regex("无效的模式捕获") to "模式错误",
        Regex("格式错误的模式") to "模式错误",
        Regex("捕获过多") to "模式错误",
        Regex("模式过于复杂") to "模式错误",
        Regex("缺少 '.+' 在模式中") to "模式错误",
        Regex("未完成的捕获") to "模式错误",
        Regex("无效使用") to "模式错误",
        Regex("无效的替换值") to "模式错误",
        Regex("'insert' 参数数量错误") to "参数错误",
        Regex("'concat' 表中") to "类型错误",
        Regex("解包结果过多") to "内存错误",
        Regex("排序的顺序函数无效") to "参数错误",
        Regex("attempt to index a (\\w+) value") to "类型错误",
        Regex("attempt to call (?:a value of type )?(.+)") to "调用错误",
        Regex("(.+?) '(.+?)' expected") to "期望值错误",
        Regex("(.+?) got (.+?), expected (.+)") to "类型不匹配",
        Regex("(.+?) is not a valid (.+)") to "无效值错误",
        Regex("module '(.+?)' not found") to "模块未找到",
        Regex("(.+?) requires argument '(.+?)'") to "参数缺失",
        Regex("布局表中缺少第一个值") to "布局缺少视图类",
        Regex("无法解析视图类") to "视图类解析失败",
        Regex("视图创建错误") to "视图创建失败"
    )

    private val solutionSuggestions = mapOf(
        "语法错误" to listOf(
            "1. 检查括号、引号是否配对",
            "2. 检查关键字拼写是否正确",
            "3. 检查语句结尾是否有分号或其他符号",
            "4. 确认字符串使用正确的引号包裹"
        ),
        "类型错误" to listOf(
            "1. 检查变量类型是否正确",
            "2. 使用 type() 函数检查变量类型",
            "3. 确保操作数类型匹配（如不能对字符串使用算术运算）"
        ),
        "调用错误" to listOf(
            "1. 检查函数名是否正确",
            "2. 确认函数是否已定义",
            "3. 检查函数参数数量和类型是否正确",
            "4. 确认函数是否已通过 import 导入"
        ),
        "布局缺少视图类" to listOf(
            "1. 确保布局表第一个元素是有效的视图类",
            "2. 使用 import 导入视图类",
            "3. 或使用完整类名字符串（如 \"android.widget.Button\"）",
            "4. 检查类名是否被截断或拼写错误"
        ),
        "视图类解析失败" to listOf(
            "1. 确认类名完整且正确",
            "2. 在布局开头添加 import 语句",
            "3. 检查是否缺少必要的 import",
            "4. 验证类是否存在于当前 ClassLoader 中"
        ),
        "视图创建失败" to listOf(
            "1. 检查视图类构造函数参数",
            "2. 确认视图类是 View 的子类",
            "3. 检查 Context 是否有效",
            "4. 检查样式资源是否存在"
        )
    )

    /**
     * 解析 Lua 错误消息
     */
    fun parseErrorMessage(errorMsg: String): Pair<String, String> {
        for ((pattern, type) in errorPatterns) {
            val match = pattern.find(errorMsg)
            if (match != null) {
                return Pair(type, match.groupValues.joinToString(" - "))
            }
        }
        return Pair("未知错误", errorMsg)
    }

    /**
     * 获取错误解决方案
     */
    fun getSolutionSuggestion(errorType: String): List<String> {
        return solutionSuggestions[errorType] ?: listOf(
            "1. 查看错误消息确定问题位置",
            "2. 检查相关代码逻辑",
            "3. 如有疑问，请联系开发者"
        )
    }

    /**
     * 获取所有解决方案
     */
    fun getAllSolutions(): Map<String, List<String>> {
        return solutionSuggestions
    }

    /**
     * 解析错误位置
     */
    fun parseErrorLocation(errorMsg: String): Triple<String?, Int?, String?> {
        val locationMatch = Regex("(.+?):(\\d+):").find(errorMsg)
        if (locationMatch != null) {
            val lineMatch = Regex("line (\\d+)").find(errorMsg)
            return Triple(
                locationMatch.groupValues[1],
                lineMatch?.groupValues?.get(1)?.toIntOrNull() ?: locationMatch.groupValues[2].toIntOrNull(),
                null
            )
        }
        return Triple(null, null, null)
    }
}

/**
 * 视图属性错误解析器
 */
object ViewAttributeErrorParser {

    private val attributeErrors = mapOf(
        "Unknown attribute" to "未知属性，请检查属性名是否正确",
        "Invalid dimension" to "尺寸值无效，请检查尺寸格式（如 16dp、match_parent）",
        "Invalid color" to "颜色值无效，请检查颜色格式（如 #FF0000、red）",
        "Class not found" to "类名无效，请检查完整类名是否正确",
        "Method not found" to "方法不存在，请检查方法名是否正确",
        "IllegalArgumentException" to "参数非法，请检查参数值是否在有效范围内",
        "NullPointerException" to "空指针，请检查对象是否已初始化",
        "IllegalStateException" to "状态非法，请检查是否在正确的生命周期调用"
    )

    /**
     * 解析属性错误
     */
    fun parseAttributeError(error: Throwable, attribute: String?): String {
        val errorMsg = error.message ?: error.toString()

        for ((pattern, suggestion) in attributeErrors) {
            if (errorMsg.contains(pattern)) {
                return buildString {
                    appendLine("属性错误: $attribute")
                    appendLine("原因: $suggestion")
                    appendLine("原始错误: $errorMsg")
                }
            }
        }

        return buildString {
            appendLine("属性错误: $attribute")
            appendLine("原始错误: $errorMsg")
        }
    }

    /**
     * 获取所有属性错误解决方案
     */
    fun getAllSuggestions(): Map<String, String> {
        return attributeErrors
    }
}
