package com.difierline.lua.lxclua.utils;

import org.json.JSONArray
import org.json.JSONObject

object JsonUtil {
    
    @JvmStatic
    fun parseObject(jsonString: String): Map<String, Any?> {
        val json = JSONObject(jsonString)
        return jsonToMap(json)
    }
    
    @JvmStatic
    fun parseArray(jsonString: String): List<Map<String, Any?>> {
        val json = JSONArray(jsonString)
        return jsonToList(json)
    }

    private fun jsonToMap(json: JSONObject): Map<String, Any?> {
        val map = mutableMapOf<String, Any?>()
        val keys = json.keys()
        while (keys.hasNext()) {
            val key = keys.next()
            val value = json.get(key)
            map[key] = gsonToJson(value)
        }
        return map
    }

    private fun jsonToList(json: JSONArray): List<Map<String, Any?>> {
        val list = mutableListOf<Map<String, Any?>>()
        for (i in 0 until json.length()) {
            val value = json.get(i)
            if (value is JSONObject) {
                list.add(jsonToMap(value))
            } else if (value is JSONArray) {
                list.add(mapOf("array" to jsonToList(value)))
            } else {
                list.add(mapOf("" to gsonToJson(value)))
            }
        }
        return list
    }

    private fun gsonToJson(value: Any): Any? {
        return when (value) {
            is JSONObject -> jsonToMap(value)
            is JSONArray -> jsonToList(value)
            else -> value
        }
    }
}