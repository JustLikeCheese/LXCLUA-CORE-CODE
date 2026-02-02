package com.difierline.lua.lxclua.utils

import java.security.MessageDigest
import java.security.SecureRandom
import javax.crypto.Cipher
import javax.crypto.spec.SecretKeySpec
import android.util.Base64
import java.util.Collections
import java.util.concurrent.ConcurrentHashMap

object AesUtil {

    // 使用并发安全的缓存
    private val keyCache = ConcurrentHashMap<String, ByteArray>()
    private const val BASE64_FLAGS = Base64.NO_WRAP
    private val secureRandom = SecureRandom()

    init {
        // 预热
        warmUp()
    }

    private fun warmUp() {
        try {
            val testKey = "warmup_key"
            val testData = "test_data"
            encryptToBase64(testKey, testData)
        } catch (e: Exception) {
            // 忽略预热异常
        }
    }

    private fun toBytes(str: String): ByteArray {
        return str.toByteArray(Charsets.UTF_8)
    }

    private fun deriveKey(passphrase: String): ByteArray {
        val md = MessageDigest.getInstance("SHA-256")
        return md.digest(toBytes(passphrase))
    }

    @Synchronized
    private fun getCachedKey(passphrase: String): ByteArray {
        return keyCache.getOrPut(passphrase) {
            deriveKey(passphrase)
        }
    }

    private fun buildCipher(opmode: Int, passphrase: String): Cipher {
        val cipher = Cipher.getInstance("AES/ECB/PKCS5Padding")
        val keySpec = SecretKeySpec(getCachedKey(passphrase), "AES")
        cipher.init(opmode, keySpec)
        return cipher
    }

    // 公共API ------------------------------------------------------------------
    fun encrypt(key: String, plainBytes: ByteArray): ByteArray {
        val cipher = buildCipher(Cipher.ENCRYPT_MODE, key)
        return cipher.doFinal(plainBytes)
    }

    fun decrypt(key: String, cipherBytes: ByteArray): ByteArray {
        val cipher = buildCipher(Cipher.DECRYPT_MODE, key)
        return cipher.doFinal(cipherBytes)
    }

    @JvmStatic
    fun encryptToBase64(key: String, plainText: String): String {
        val bytes = encrypt(key, toBytes(plainText))
        return Base64.encodeToString(bytes, BASE64_FLAGS)
    }

    @JvmStatic
    fun decryptFromBase64(key: String, base64Text: String): String {
        val bytes = Base64.decode(base64Text, BASE64_FLAGS)
        val plain = decrypt(key, bytes)
        return String(plain, Charsets.UTF_8)
    }

    // 批量操作API
    @JvmStatic
    fun encryptMultiple(key: String, plainTexts: List<String>): List<String> {
        val cipher = buildCipher(Cipher.ENCRYPT_MODE, key)
        return plainTexts.map { plainText ->
            val bytes = cipher.doFinal(toBytes(plainText))
            Base64.encodeToString(bytes, BASE64_FLAGS)
        }
    }

    @JvmStatic
    fun decryptMultiple(key: String, base64Texts: List<String>): List<String> {
        val cipher = buildCipher(Cipher.DECRYPT_MODE, key)
        return base64Texts.map { base64Text ->
            val bytes = Base64.decode(base64Text, BASE64_FLAGS)
            String(cipher.doFinal(bytes), Charsets.UTF_8)
        }
    }
}