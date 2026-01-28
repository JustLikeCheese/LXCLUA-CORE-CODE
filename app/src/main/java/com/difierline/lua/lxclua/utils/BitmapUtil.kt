package com.difierline.lua.lxclua.utils

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.util.Log
import android.util.LruCache
import com.difierline.lua.lxclua.utils.exceptions.BitmapLoadException
import java.io.*
import java.net.HttpURLConnection
import java.net.URL
import java.security.MessageDigest
import java.util.concurrent.*

class BitmapUtil private constructor() {

    companion object {

        private const val TAG = "BitmapUtil"

        /* ---------- 内存缓存 ---------- */
        private val memCache = object : LruCache<String, Bitmap>((maxMemory() / 8).toInt()) {
            override fun sizeOf(key: String, bmp: Bitmap): Int = bmp.byteCount
        }

        /* ---------- 磁盘缓存目录 ---------- */
        private val diskDir: File by lazy {
            File(appContext().cacheDir, "bitmap_cache").also { dir ->
                if (!dir.exists()) dir.mkdirs()
            }
        }

        /* ---------- 线程池 ---------- */
        private val pool = Executors.newFixedThreadPool(4)

        /* ---------- 公开方法 ---------- */

        /**
         * 加载网络图片（永远优先缓存）
         */
        @JvmStatic
        @JvmOverloads
        @Throws(BitmapLoadException::class)
        fun fromUrl(
            url: String,
            connectTimeout: Int = 10_000,
            readTimeout: Int = 15_000
        ): Bitmap {
            val key = md5(url)

            // 1. 内存
            memCache.get(key)?.let { return it }

            // 2. 磁盘
            diskFile(key).takeIf { it.exists() }?.let { file ->
                BitmapFactory.decodeFile(file.absolutePath)?.also { bmp ->
                    memCache.put(key, bmp)
                    return bmp
                }
            }

            // 3. 网络
            val bmp = downloadBitmap(url, connectTimeout, readTimeout)
            saveToCache(key, bmp)
            return bmp
        }

        /**
         * 加载本地文件（永远优先缓存）
         */
        @JvmStatic
        @Throws(BitmapLoadException::class)
        fun fromFile(path: String): Bitmap {
            val key = md5(path)

            // 1. 内存
            memCache.get(key)?.let { return it }

            // 2. 磁盘
            diskFile(key).takeIf { it.exists() }?.let { file ->
                BitmapFactory.decodeFile(file.absolutePath)?.also { bmp ->
                    memCache.put(key, bmp)
                    return bmp
                }
            }

            // 3. 文件
            val bmp = BitmapFactory.decodeFile(path)
                ?: throw BitmapLoadException("decodeFile null")
            saveToCache(key, bmp)
            return bmp
        }

        @JvmStatic
        fun fromFile(file: File): Bitmap = fromFile(file.absolutePath)

        /* ---------- 清理缓存 ---------- */

        @JvmStatic
        fun clearCache() {
            memCache.evictAll()
            diskDir.listFiles()?.forEach { it.delete() }
        }

        /* ---------- 私有实现 ---------- */

        private fun maxMemory() = Runtime.getRuntime().maxMemory()

        private fun appContext() =
            Class.forName("android.app.ActivityThread")
                .getMethod("currentApplication")
                .invoke(null) as android.app.Application

        private fun diskFile(key: String) = File(diskDir, key)

        private fun saveToCache(key: String, bmp: Bitmap) {
            // 内存
            memCache.put(key, bmp)
            // 磁盘
            FileOutputStream(diskFile(key)).use {
                bmp.compress(Bitmap.CompressFormat.PNG, 100, it)
            }
        }

        private fun downloadBitmap(
            url: String,
            connectTimeout: Int,
            readTimeout: Int
        ): Bitmap {
            val future = pool.submit(Callable {
                var conn: HttpURLConnection? = null
                try {
                    conn = (URL(url).openConnection() as HttpURLConnection).apply {
                        this.connectTimeout = connectTimeout
                        this.readTimeout = readTimeout
                        requestMethod = "GET"
                        setRequestProperty("User-Agent", "Android/BitmapUtil")
                    }
                    if (conn.responseCode != HttpURLConnection.HTTP_OK) {
                        throw BitmapLoadException("HTTP ${conn.responseCode}")
                    }
                    conn.inputStream.use {
                        BitmapFactory.decodeStream(it)
                            ?: throw BitmapLoadException("decodeStream null")
                    }
                } catch (e: Exception) {
                    throw if (e is BitmapLoadException) e else BitmapLoadException(e.message ?: "", e)
                } finally {
                    conn?.disconnect()
                }
            })

            val totalTimeout = (connectTimeout + readTimeout + 5000L)
            return try {
                future.get(totalTimeout, TimeUnit.MILLISECONDS)
            } catch (e: Exception) {
                future.cancel(true)
                throw BitmapLoadException("timeout/interrupt", e)
            }
        }

        private fun md5(input: String): String {
            val bytes = MessageDigest.getInstance("MD5").digest(input.toByteArray())
            return bytes.joinToString("") { "%02x".format(it) }
        }
    }
}
