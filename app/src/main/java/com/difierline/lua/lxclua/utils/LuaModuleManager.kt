package com.difierline.lua.lxclua.utils

import android.content.Context
import android.content.SharedPreferences
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.RectF
import android.os.Handler
import android.os.Looper
import android.util.Base64
import android.util.Log
import android.view.View
import com.difierline.lua.lxclua.utils.exceptions.LuaErrorHandler
import java.io.ByteArrayOutputStream
import java.io.File
import java.io.FileInputStream
import java.io.FileOutputStream
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit

/**
 * Lua 工具模块 - 提供 OOP 风格的工具类供 Lua 调用
 */
object LuaModuleRegistry {

    private val modules = mutableMapOf<String, Any?>()

    fun register(name: String, module: Any?) {
        modules[name] = module
    }

    fun get(name: String): Any? = modules[name]

    fun getOrCreate(name: String, creator: () -> Any?): Any? {
        return modules.getOrPut(name) { creator() }
    }
}

/**
 * 异步任务管理器
 */
class AsyncTaskManager private constructor() {

    companion object {
        @Volatile
        private var instance: AsyncTaskManager? = null
        private val mainHandler = Handler(Looper.getMainLooper())
        private val executor: ExecutorService = Executors.newCachedThreadPool()

        fun getInstance(): AsyncTaskManager {
            return instance ?: synchronized(this) {
                instance ?: AsyncTaskManager().also { instance = it }
            }
        }
    }

    /**
     * 在后台线程执行任务
     */
    fun <T> execute(
        task: () -> T,
        onSuccess: ((T) -> Unit)? = null,
        onError: ((Throwable) -> Unit)? = null
    ): Int {
        val taskId = System.currentTimeMillis().toInt()
        executor.execute {
            try {
                val result = task()
                mainHandler.post {
                    onSuccess?.invoke(result)
                }
            } catch (e: Exception) {
                mainHandler.post {
                    onError?.invoke(e)
                }
            }
        }
        return taskId
    }

    /**
     * 在主线程执行
     */
    fun runOnMain(runnable: () -> Unit) {
        mainHandler.post(runnable)
    }

    /**
     * 延迟执行
     */
    fun postDelayed(runnable: () -> Unit, delayMs: Long): Int {
        val taskId = System.currentTimeMillis().toInt()
        mainHandler.postDelayed(runnable, delayMs)
        return taskId
    }

    /**
     * 取消延迟任务
     */
    fun cancelDelayed(taskId: Int) {
        mainHandler.removeCallbacksAndMessages(taskId)
    }

    /**
     * 关闭线程池
     */
    fun shutdown() {
        executor.shutdown()
        try {
            if (!executor.awaitTermination(1, TimeUnit.SECONDS)) {
                executor.shutdownNow()
            }
        } catch (e: InterruptedException) {
            executor.shutdownNow()
        }
    }
}

/**
 * 计时器类
 */
class Timer private constructor() {

    companion object {
        @Volatile
        private var instance: Timer? = null
        private val handler = Handler(Looper.getMainLooper())

        fun getInstance(): Timer {
            return instance ?: synchronized(this) {
                instance ?: Timer().also { instance = it }
            }
        }
    }

    private val timers = mutableMapOf<Int, Runnable>()

    /**
     * 开始一个定时器
     * @param intervalMs 间隔毫秒
     * @param callback 回调函数
     * @param repeat 是否重复
     * @return 定时器 ID
     */
    fun start(
        intervalMs: Long,
        callback: (Int) -> Unit,
        repeat: Boolean = true
    ): Int {
        val timerId = System.currentTimeMillis().toInt()
        val runnable = object : Runnable {
            override fun run() {
                callback(timerId)
                if (repeat) {
                    handler.postDelayed(this, intervalMs)
                }
            }
        }
        timers[timerId] = runnable
        handler.post(runnable)
        return timerId
    }

    /**
     * 停止定时器
     */
    fun stop(timerId: Int) {
        timers[timerId]?.let {
            handler.removeCallbacks(it)
            timers.remove(timerId)
        }
    }

    /**
     * 停止所有定时器
     */
    fun stopAll() {
        timers.values.forEach { handler.removeCallbacks(it) }
        timers.clear()
    }
}

/**
 * 文件操作类
 */
class FileUtil private constructor() {

    companion object {
        @Volatile
        private var instance: FileUtil? = null

        fun getInstance(): FileUtil {
            return instance ?: synchronized(this) {
                instance ?: FileUtil().also { instance = it }
            }
        }
    }

    /**
     * 读取文件内容
     */
    fun read(path: String): String? {
        return try {
            File(path).readText()
        } catch (e: Exception) {
            Log.e("FileUtil", "读取文件失败: $path", e)
            null
        }
    }

    /**
     * 写入文件内容
     */
    fun write(path: String, content: String): Boolean {
        return try {
            File(path).parentFile?.mkdirs()
            File(path).writeText(content)
            true
        } catch (e: Exception) {
            Log.e("FileUtil", "写入文件失败: $path", e)
            false
        }
    }

    /**
     * 追加文件内容
     */
    fun append(path: String, content: String): Boolean {
        return try {
            File(path).parentFile?.mkdirs()
            File(path).appendText(content)
            true
        } catch (e: Exception) {
            Log.e("FileUtil", "追加文件失败: $path", e)
            false
        }
    }

    /**
     * 检查文件是否存在
     */
    fun exists(path: String): Boolean {
        return File(path).exists()
    }

    /**
     * 删除文件
     */
    fun delete(path: String): Boolean {
        return try {
            File(path).delete()
        } catch (e: Exception) {
            Log.e("FileUtil", "删除文件失败: $path", e)
            false
        }
    }

    /**
     * 复制文件
     */
    fun copy(src: String, dest: String): Boolean {
        return try {
            File(src).copyTo(File(dest), overwrite = true)
            true
        } catch (e: Exception) {
            Log.e("FileUtil", "复制文件失败: $src -> $dest", e)
            false
        }
    }

    /**
     * 移动文件
     */
    fun move(src: String, dest: String): Boolean {
        return try {
            File(src).copyTo(File(dest), overwrite = true)
            File(src).delete()
            true
        } catch (e: Exception) {
            Log.e("FileUtil", "移动文件失败: $src -> $dest", e)
            false
        }
    }

    /**
     * 获取文件大小
     */
    fun size(path: String): Long {
        return try {
            File(path).length()
        } catch (e: Exception) {
            0L
        }
    }

    /**
     * 创建目录
     */
    fun mkdir(path: String): Boolean {
        return try {
            File(path).mkdirs()
            true
        } catch (e: Exception) {
            Log.e("FileUtil", "创建目录失败: $path", e)
            false
        }
    }

    /**
     * 列出目录文件
     */
    fun listFiles(path: String): List<String> {
        return try {
            File(path).listFiles()?.map { it.absolutePath } ?: emptyList()
        } catch (e: Exception) {
            emptyList()
        }
    }
}

/**
 * SharedPreferences 封装类
 */
class Prefs private constructor(context: Context) {

    companion object {
        @Volatile
        private var instance: Prefs? = null

        fun getInstance(context: Context): Prefs {
            return instance ?: synchronized(this) {
                instance ?: Prefs(context.applicationContext).also { instance = it }
            }
        }
    }

    private val context: Context = context
    private val prefsMap = mutableMapOf<String, SharedPreferences>()

    private fun getPrefs(name: String): SharedPreferences {
        return prefsMap.getOrPut(name) {
            context.getSharedPreferences(name, Context.MODE_PRIVATE)
        }
    }

    fun putString(name: String, key: String, value: String) {
        getPrefs(name).edit().putString(key, value).apply()
    }

    fun getString(name: String, key: String, default: String = ""): String {
        return getPrefs(name).getString(key, default) ?: default
    }

    fun putInt(name: String, key: String, value: Int) {
        getPrefs(name).edit().putInt(key, value).apply()
    }

    fun getInt(name: String, key: String, default: Int = 0): Int {
        return getPrefs(name).getInt(key, default)
    }

    fun putLong(name: String, key: String, value: Long) {
        getPrefs(name).edit().putLong(key, value).apply()
    }

    fun getLong(name: String, key: String, default: Long = 0L): Long {
        return getPrefs(name).getLong(key, default)
    }

    fun putFloat(name: String, key: String, value: Float) {
        getPrefs(name).edit().putFloat(key, value).apply()
    }

    fun getFloat(name: String, key: String, default: Float = 0f): Float {
        return getPrefs(name).getFloat(key, default)
    }

    fun putBoolean(name: String, key: String, value: Boolean) {
        getPrefs(name).edit().putBoolean(key, value).apply()
    }

    fun getBoolean(name: String, key: String, default: Boolean = false): Boolean {
        return getPrefs(name).getBoolean(key, default)
    }

    fun remove(name: String, key: String) {
        getPrefs(name).edit().remove(key).apply()
    }

    fun clear(name: String) {
        getPrefs(name).edit().clear().apply()
    }

    fun contains(name: String, key: String): Boolean {
        return getPrefs(name).contains(key)
    }
}

/**
 * 图片处理类
 */
class ImageUtil private constructor() {

    companion object {
        @Volatile
        private var instance: ImageUtil? = null

        fun getInstance(): ImageUtil {
            return instance ?: synchronized(this) {
                instance ?: ImageUtil().also { instance = it }
            }
        }
    }

    /**
     * 加载图片
     */
    fun load(path: String, reqWidth: Int = 0, reqHeight: Int = 0): Bitmap? {
        return try {
            if (reqWidth > 0 && reqHeight > 0) {
                val options = BitmapFactory.Options().apply {
                    inJustDecodeBounds = true
                }
                BitmapFactory.decodeFile(path, options)
                options.inSampleSize = calculateInSampleSize(options, reqWidth, reqHeight)
                options.inJustDecodeBounds = false
                BitmapFactory.decodeFile(path, options)
            } else {
                BitmapFactory.decodeFile(path)
            }
        } catch (e: Exception) {
            Log.e("ImageUtil", "加载图片失败: $path", e)
            null
        }
    }

    private fun calculateInSampleSize(
        options: BitmapFactory.Options,
        reqWidth: Int,
        reqHeight: Int
    ): Int {
        val height = options.outHeight
        val width = options.outWidth
        var inSampleSize = 1

        if (height > reqHeight || width > reqWidth) {
            val halfHeight = height / 2
            val halfWidth = width / 2
            while ((halfHeight / inSampleSize) >= reqHeight &&
                (halfWidth / inSampleSize) >= reqWidth
            ) {
                inSampleSize *= 2
            }
        }
        return inSampleSize
    }

    /**
     * 保存图片
     */
    fun save(bitmap: Bitmap, path: String, quality: Int = 90): Boolean {
        return try {
            File(path).parentFile?.mkdirs()
            FileOutputStream(path).use { out ->
                val format = when {
                    path.endsWith(".png") -> Bitmap.CompressFormat.PNG
                    else -> Bitmap.CompressFormat.JPEG
                }
                bitmap.compress(format, quality, out)
            }
            true
        } catch (e: Exception) {
            Log.e("ImageUtil", "保存图片失败: $path", e)
            false
        }
    }

    /**
     * 缩放图片
     */
    fun scale(bitmap: Bitmap, scaleWidth: Float, scaleHeight: Float): Bitmap {
        val width = (bitmap.width * scaleWidth).toInt()
        val height = (bitmap.height * scaleHeight).toInt()
        return Bitmap.createScaledBitmap(bitmap, width, height, true)
    }

    /**
     * 裁剪图片
     */
    fun crop(bitmap: Bitmap, x: Int, y: Int, width: Int, height: Int): Bitmap {
        return Bitmap.createBitmap(bitmap, x, y, width, height)
    }

    /**
     * 旋转图片
     */
    fun rotate(bitmap: Bitmap, degrees: Float): Bitmap {
        val matrix = android.graphics.Matrix().apply {
            postRotate(degrees)
        }
        return Bitmap.createBitmap(bitmap, 0, 0, bitmap.width, bitmap.height, matrix, true)
    }

    /**
     * 图片转 Base64
     */
    fun toBase64(bitmap: Bitmap, quality: Int = 90): String? {
        return try {
            val stream = ByteArrayOutputStream()
            bitmap.compress(Bitmap.CompressFormat.JPEG, quality, stream)
            Base64.encodeToString(stream.toByteArray(), Base64.NO_WRAP)
        } catch (e: Exception) {
            null
        }
    }

    /**
     * Base64 转图片
     */
    fun fromBase64(base64: String): Bitmap? {
        return try {
            val bytes = Base64.decode(base64, Base64.DEFAULT)
            BitmapFactory.decodeByteArray(bytes, 0, bytes.size)
        } catch (e: Exception) {
            null
        }
    }
}

/**
 * 日期时间工具类
 */
class DateUtil private constructor() {

    companion object {
        @Volatile
        private var instance: DateUtil? = null

        fun getInstance(): DateUtil {
            return instance ?: synchronized(this) {
                instance ?: DateUtil().also { instance = it }
            }
        }
    }

    private val dateFormat = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault())
    private val dateOnlyFormat = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault())
    private val timeOnlyFormat = SimpleDateFormat("HH:mm:ss", Locale.getDefault())

    fun format(timestamp: Long, pattern: String = "yyyy-MM-dd HH:mm:ss"): String {
        return try {
            SimpleDateFormat(pattern, Locale.getDefault()).format(Date(timestamp))
        } catch (e: Exception) {
            ""
        }
    }

    fun formatDate(timestamp: Long): String = dateOnlyFormat.format(Date(timestamp))

    fun formatTime(timestamp: Long): String = timeOnlyFormat.format(Date(timestamp))

    fun formatDateTime(timestamp: Long): String = dateFormat.format(Date(timestamp))

    fun parse(dateStr: String, pattern: String = "yyyy-MM-dd HH:mm:ss"): Long? {
        return try {
            SimpleDateFormat(pattern, Locale.getDefault()).parse(dateStr)?.time
        } catch (e: Exception) {
            null
        }
    }

    fun now(): Long = System.currentTimeMillis()

    fun timestamp(): Long = System.currentTimeMillis()

    fun year(timestamp: Long = now()): Int {
        return SimpleDateFormat("yyyy", Locale.getDefault()).format(Date(timestamp)).toInt()
    }

    fun month(timestamp: Long = now()): Int {
        return SimpleDateFormat("MM", Locale.getDefault()).format(Date(timestamp)).toInt()
    }

    fun day(timestamp: Long = now()): Int {
        return SimpleDateFormat("dd", Locale.getDefault()).format(Date(timestamp)).toInt()
    }

    fun hour(timestamp: Long = now()): Int {
        return SimpleDateFormat("HH", Locale.getDefault()).format(Date(timestamp)).toInt()
    }

    fun minute(timestamp: Long = now()): Int {
        return SimpleDateFormat("mm", Locale.getDefault()).format(Date(timestamp)).toInt()
    }

    fun second(timestamp: Long = now()): Int {
        return SimpleDateFormat("ss", Locale.getDefault()).format(Date(timestamp)).toInt()
    }

    fun dayOfWeek(timestamp: Long = now()): Int {
        val cal = java.util.Calendar.getInstance()
        cal.timeInMillis = timestamp
        return cal.get(java.util.Calendar.DAY_OF_WEEK)
    }

    fun isToday(timestamp: Long): Boolean {
        val today = dateOnlyFormat.format(Date(now()))
        val target = dateOnlyFormat.format(Date(timestamp))
        return today == target
    }
}

/**
 * 字符串工具类
 */
class StringUtil private constructor() {

    companion object {
        @Volatile
        private var instance: StringUtil? = null

        fun getInstance(): StringUtil {
            return instance ?: synchronized(this) {
                instance ?: StringUtil().also { instance = it }
            }
        }
    }

    fun isEmpty(str: String?): Boolean = str.isNullOrBlank()

    fun isNotEmpty(str: String?): Boolean = !isEmpty(str)

    fun trim(str: String?): String = str?.trim() ?: ""

    fun length(str: String?): Int = str?.length ?: 0

    fun upperCase(str: String): String = str.uppercase(Locale.getDefault())

    fun lowerCase(str: String): String = str.lowercase(Locale.getDefault())

    fun capitalize(str: String): String {
        if (str.isEmpty()) return str
        return str[0].uppercaseChar() + str.substring(1).lowercase()
    }

    fun contains(str: String, searchStr: String): Boolean = str.contains(searchStr)

    fun startsWith(str: String, prefix: String): Boolean = str.startsWith(prefix)

    fun endsWith(str: String, suffix: String): Boolean = str.endsWith(suffix)

    fun replace(str: String, oldStr: String, newStr: String): String = str.replace(oldStr, newStr)

    fun replaceAll(str: String, regex: String, newStr: String): String =
        str.replace(Regex(regex), newStr)

    fun split(str: String, delimiter: String): List<String> =
        str.split(delimiter).filter { it.isNotEmpty() }

    fun join(list: List<String>, delimiter: String): String = list.joinToString(delimiter)

    fun substring(str: String, start: Int, end: Int = str.length): String {
        val safeStart = start.coerceAtLeast(0)
        val safeEnd = end.coerceAtMost(str.length)
        if (safeStart >= safeEnd) return ""
        return str.substring(safeStart, safeEnd)
    }

    fun substringAfter(str: String, delimiter: String): String {
        val index = str.indexOf(delimiter)
        return if (index >= 0) str.substring(index + delimiter.length) else ""
    }

    fun substringBefore(str: String, delimiter: String): String {
        val index = str.indexOf(delimiter)
        return if (index >= 0) str.substring(0, index) else str
    }

    fun format(template: String, vararg args: Any): String {
        return template.replace(Regex("%\\{([^}]+)\\}")) { match ->
            val key = match.groupValues[1]
            args.getOrNull(key.toIntOrNull() ?: 0)?.toString() ?: match.value
        }
    }

    fun padLeft(str: String, length: Int, padChar: Char = ' '): String {
        return str.padStart(length, padChar)
    }

    fun padRight(str: String, length: Int, padChar: Char = ' '): String {
        return str.padEnd(length, padChar)
    }

    fun repeat(str: String, count: Int): String = str.repeat(count.coerceAtLeast(0))

    fun reverse(str: String): String = str.reversed()

    fun uuid(): String = java.util.UUID.randomUUID().toString()

    fun md5(str: String): String {
        val md = java.security.MessageDigest.getInstance("MD5")
        val digest = md.digest(str.toByteArray())
        return digest.joinToString("") { "%02x".format(it) }
    }
}

/**
 * 颜色工具类
 */
class ColorUtil private constructor() {

    companion object {
        @Volatile
        private var instance: ColorUtil? = null

        fun getInstance(): ColorUtil {
            return instance ?: synchronized(this) {
                instance ?: ColorUtil().also { instance = it }
            }
        }
    }

    fun parse(colorStr: String): Int? {
        return try {
            Color.parseColor(colorStr)
        } catch (e: Exception) {
            null
        }
    }

    fun rgb(red: Int, green: Int, blue: Int): Int {
        return Color.rgb(red.coerceIn(0, 255), green.coerceIn(0, 255), blue.coerceIn(0, 255))
    }

    fun rgba(red: Int, green: Int, blue: Int, alpha: Int): Int {
        return Color.argb(alpha.coerceIn(0, 255), red.coerceIn(0, 255), green.coerceIn(0, 255), blue.coerceIn(0, 255))
    }

    fun getRed(color: Int): Int = Color.red(color)

    fun getGreen(color: Int): Int = Color.green(color)

    fun getBlue(color: Int): Int = Color.blue(color)

    fun getAlpha(color: Int): Int = Color.alpha(color)

    fun setAlpha(color: Int, alpha: Int): Int {
        return Color.argb(alpha.coerceIn(0, 255), Color.red(color), Color.green(color), Color.blue(color))
    }

    fun toHex(color: Int, includeAlpha: Boolean = true): String {
        return if (includeAlpha) {
            String.format("#%08X", color)
        } else {
            String.format("#%06X", 0xFFFFFF and color)
        }
    }

    fun lighten(color: Int, factor: Float): Int {
        val hsv = FloatArray(3)
        Color.colorToHSV(color, hsv)
        hsv[2] = (hsv[2] + factor).coerceIn(0f, 1f)
        return Color.HSVToColor(hsv)
    }

    fun darken(color: Int, factor: Float): Int {
        val hsv = FloatArray(3)
        Color.colorToHSV(color, hsv)
        hsv[2] = (hsv[2] - factor).coerceIn(0f, 1f)
        return Color.HSVToColor(hsv)
    }

    fun mix(color1: Int, color2: Int, ratio: Float): Int {
        val inverseRatio = 1f - ratio.coerceIn(0f, 1f)
        val r = (Color.red(color1) * inverseRatio + Color.red(color2) * ratio).toInt()
        val g = (Color.green(color1) * inverseRatio + Color.green(color2) * ratio).toInt()
        val b = (Color.blue(color1) * inverseRatio + Color.blue(color2) * ratio).toInt()
        return Color.rgb(r, g, b)
    }

    fun isLight(color: Int): Boolean {
        val luminance = (0.299 * Color.red(color) + 0.587 * Color.green(color) + 0.114 * Color.blue(color)) / 255
        return luminance > 0.5
    }
}

/**
 * 事件总线 - 用于组件间通信
 */
class EventBus private constructor() {

    companion object {
        @Volatile
        private var instance: EventBus? = null

        fun getInstance(): EventBus {
            return instance ?: synchronized(this) {
                instance ?: EventBus().also { instance = it }
            }
        }
    }

    private val handlers = mutableMapOf<String, MutableList<(Any?) -> Unit>>()

    /**
     * 订阅事件
     */
    fun subscribe(event: String, handler: (Any?) -> Unit) {
        handlers.getOrPut(event) { mutableListOf() }.add(handler)
    }

    /**
     * 取消订阅
     */
    fun unsubscribe(event: String, handler: ((Any?) -> Unit)? = null) {
        if (handler != null) {
            handlers[event]?.remove(handler)
        } else {
            handlers.remove(event)
        }
    }

    /**
     * 发布事件
     */
    fun post(event: String, data: Any? = null) {
        handlers[event]?.forEach { it(data) }
    }

    /**
     * 清除所有订阅
     */
    fun clear() {
        handlers.clear()
    }
}

/**
 * 缓存管理器
 */
class CacheManager private constructor() {

    companion object {
        @Volatile
        private var instance: CacheManager? = null

        fun getInstance(): CacheManager {
            return instance ?: synchronized(this) {
                instance ?: CacheManager().also { instance = it }
            }
        }
    }

    private val memoryCache = object : LinkedHashMap<String, Any?>(
        100, 0.75f, true
    ) {
        override fun removeEldestEntry(eldest: MutableMap.MutableEntry<String, Any?>?): Boolean {
            return size > 100
        }
    }

    private val maxSize = 100

    /**
     * 设置缓存
     */
    fun put(key: String, value: Any?) {
        synchronized(memoryCache) {
            memoryCache[key] = value
        }
    }

    /**
     * 获取缓存
     */
    fun get(key: String): Any? {
        return synchronized(memoryCache) {
            memoryCache[key]
        }
    }

    /**
     * 获取缓存并转换为指定类型
     */
    inline fun <reified T> getAs(key: String): T? {
        return get(key) as? T
    }

    /**
     * 移除缓存
     */
    fun remove(key: String): Any? {
        return synchronized(memoryCache) {
            memoryCache.remove(key)
        }
    }

    /**
     * 清空缓存
     */
    fun clear() {
        synchronized(memoryCache) {
            memoryCache.clear()
        }
    }

    /**
     * 检查缓存是否存在
     */
    fun contains(key: String): Boolean {
        return synchronized(memoryCache) {
            memoryCache.containsKey(key)
        }
    }

    /**
     * 获取缓存大小
     */
    fun size(): Int = memoryCache.size
}

/**
 * 视图扩展工具
 */
object ViewUtils {

    /**
     * 设置视图可见性
     */
    fun setVisibility(view: View, visible: Boolean) {
        view.visibility = if (visible) View.VISIBLE else View.GONE
    }

    /**
     * 设置视图可见性（可隐藏）
     */
    fun setVisibility(view: View, visible: Boolean, invisible: Boolean = false) {
        view.visibility = when {
            visible -> View.VISIBLE
            invisible -> View.INVISIBLE
            else -> View.GONE
        }
    }

    /**
     * 切换视图可见性
     */
    fun toggleVisibility(view: View) {
        view.visibility = when (view.visibility) {
            View.VISIBLE -> View.GONE
            else -> View.VISIBLE
        }
    }

    /**
     * 设置视图宽高
     */
    fun setSize(view: View, width: Int, height: Int) {
        view.layoutParams = view.layoutParams.apply {
            this.width = width
            this.height = height
        }
    }

    /**
     * 设置视图边距
     */
    fun setMargin(view: View, left: Int = 0, top: Int = 0, right: Int = 0, bottom: Int = 0) {
        view.layoutParams = (view.layoutParams as? android.view.ViewGroup.MarginLayoutParams)?.apply {
            setMargins(left, top, right, bottom)
        }
    }

    /**
     * 设置视图内边距
     */
    fun setPadding(view: View, left: Int = 0, top: Int = 0, right: Int = 0, bottom: Int = 0) {
        view.setPadding(left, top, right, bottom)
    }

    /**
     * 测量视图
     */
    fun measure(view: View, widthSpec: Int = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED),
                heightSpec: Int = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED)): Pair<Int, Int> {
        view.measure(widthSpec, heightSpec)
        return Pair(view.measuredWidth, view.measuredHeight)
    }

    /**
     * 获取视图位置
     */
    fun getLocation(view: View): Pair<Int, Int> {
        val location = IntArray(2)
        view.getLocationOnScreen(location)
        return Pair(location[0], location[1])
    }

    /**
     * 在视图中绘制
     */
    fun drawOnView(view: View, draw: (Canvas, Paint) -> Unit) {
        val bitmap = Bitmap.createBitmap(view.width, view.height, Bitmap.Config.ARGB_8888)
        val canvas = Canvas(bitmap)
        val paint = Paint().apply {
            isAntiAlias = true
        }
        draw(canvas, paint)
        view.setBackground(android.graphics.drawable.BitmapDrawable(view.context.resources, bitmap))
    }
}

/**
 * 日志工具类
 */
object Logger {

    private const val DEFAULT_TAG = "LXCLUA"

    fun d(tag: String = DEFAULT_TAG, message: String) {
        Log.d(tag, message)
    }

    fun i(tag: String = DEFAULT_TAG, message: String) {
        Log.i(tag, message)
    }

    fun w(tag: String = DEFAULT_TAG, message: String) {
        Log.w(tag, message)
    }

    fun e(tag: String = DEFAULT_TAG, message: String, throwable: Throwable? = null) {
        if (throwable != null) {
            Log.e(tag, message, throwable)
        } else {
            Log.e(tag, message)
        }
    }

    fun v(tag: String = DEFAULT_TAG, message: String) {
        Log.v(tag, message)
    }

    fun json(tag: String = DEFAULT_TAG, json: String) {
        val formatted = json.replace(Regex("(\\{|\\[|,)"), "$1\n  ")
            .replace(Regex("\\}"), "\n}")
            .replace(Regex("\\n  \\n"), "\n  ")
        Log.d(tag, formatted)
    }
}
