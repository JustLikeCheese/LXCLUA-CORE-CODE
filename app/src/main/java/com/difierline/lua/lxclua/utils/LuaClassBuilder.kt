package com.difierline.lua.lxclua.utils

import java.lang.reflect.*
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.CopyOnWriteArrayList
import kotlin.math.*

object LuaClassBuilder {
    
    private val classCache = ConcurrentHashMap<String, Class<*>>()
    private val methodCache = ConcurrentHashMap<String, MutableMap<String, Method>>()
    private val constructorCache = ConcurrentHashMap<String, Constructor<*>>()
    
    @JvmStatic
    @Suppress("UNCHECKED_CAST")
    fun <T> createClass(className: String, fields: Map<String, Any>): T {
        return SimpleDataClass(fields) as T
    }

    @JvmStatic
    @Suppress("UNCHECKED_CAST")
    fun <T> createClassWithArgs(className: String, vararg fieldPairs: Any): T {
        val fields = mutableMapOf<String, Any>()
        var i = 0
        while (i < fieldPairs.size - 1) {
            fields[fieldPairs[i].toString()] = fieldPairs[i + 1]
            i += 2
        }
        return SimpleDataClass(fields) as T
    }
    
    @JvmStatic
    fun createClassWithSupers(
        className: String,
        fields: Map<String, Any>,
        superClass: Map<String, Any> = emptyMap(),
        interfaces: List<Map<String, Any>> = emptyList()
    ): Any {
        return SimpleDataClass(fields + superClass)
    }
    
    @JvmStatic
    fun createDataClass(className: String, vararg fieldPairs: Any): Any {
        val fields = mutableMapOf<String, Any>()
        var i = 0
        while (i < fieldPairs.size - 1) {
            fields[fieldPairs[i].toString()] = fieldPairs[i + 1]
            i += 2
        }
        return SimpleDataClass(fields)
    }
    
    @JvmStatic
    @Suppress("UNCHECKED_CAST")
    fun <T> newInstance(className: String, vararg constructorArgs: Any): T {
        return try {
            val clazz = loadOrDefineClass(className)
            val argTypes = constructorArgs.map { it::class.java }.toTypedArray()
            val constructor = clazz.getDeclaredConstructor(*argTypes)
            constructor.isAccessible = true
            constructor.newInstance(*constructorArgs) as T
        } catch (e: Exception) {
            SimpleDataClass(emptyMap()) as T
        }
    }
    
    private fun loadOrDefineClass(className: String): Class<*> {
        return try {
            Class.forName("com.difierline.lua.lxclua.utils.$className")
        } catch (e: Exception) {
            Class.forName("java.lang.Object")
        }
    }
    
    @JvmStatic
    fun getClass(obj: Any): Class<*> = obj::class.java
    
    @JvmStatic
    fun getClassName(obj: Any): String = obj::class.java.simpleName
    
    @JvmStatic
    fun getClassFullName(obj: Any): String = obj::class.java.name
    
    @JvmStatic
    fun isInstance(obj: Any, className: String): Boolean {
        return try {
            val targetClass = Class.forName("com.difierline.lua.lxclua.utils.$className")
            targetClass.isInstance(obj)
        } catch (e: Exception) {
            false
        }
    }
    
    @JvmStatic
    fun getField(obj: Any, fieldName: String): Any? {
        return try {
            val field = obj::class.java.getDeclaredField(fieldName)
            field.isAccessible = true
            field.get(obj)
        } catch (e: Exception) {
            null
        }
    }
    
    @JvmStatic
    fun setField(obj: Any, fieldName: String, value: Any) {
        try {
            val field = obj::class.java.getDeclaredField(fieldName)
            field.isAccessible = true
            field.set(obj, convertValue(value, field.type))
        } catch (e: Exception) {
        }
    }
    
    @JvmStatic
    fun getFieldType(obj: Any, fieldName: String): String {
        return try {
            val field = obj::class.java.getDeclaredField(fieldName)
            field.type.name
        } catch (e: Exception) {
            "unknown"
        }
    }
    
    @JvmStatic
    fun hasField(obj: Any, fieldName: String): Boolean {
        return try {
            obj::class.java.getDeclaredField(fieldName)
            true
        } catch (e: NoSuchFieldException) {
            false
        }
    }
    
    @JvmStatic
    fun getAllFields(obj: Any): List<String> {
        return obj::class.java.declaredFields.filter { !Modifier.isStatic(it.modifiers) }.map { it.name }
    }
    
    @JvmStatic
    fun getAllFieldValues(obj: Any): Map<String, Any> {
        val result = mutableMapOf<String, Any>()
        try {
            for (field in obj::class.java.declaredFields) {
                if (!Modifier.isStatic(field.modifiers)) {
                    field.isAccessible = true
                    result[field.name] = field.get(obj) ?: "null"
                }
            }
        } catch (e: Exception) {
        }
        return result
    }
    
    @JvmStatic
    fun callMethod(obj: Any, methodName: String, vararg args: Any): Any? {
        return try {
            val argTypes = args.map { 
                when (it) {
                    is Int -> Integer.TYPE
                    is Long -> java.lang.Long.TYPE
                    is Float -> java.lang.Float.TYPE
                    is Double -> java.lang.Double.TYPE
                    is Boolean -> java.lang.Boolean.TYPE
                    else -> it::class.java
                }
            }.toTypedArray()
            val method = findMethod(obj::class.java, methodName, argTypes)
            method?.invoke(obj, *args)
        } catch (e: Exception) {
            null
        }
    }
    
    private fun findMethod(clazz: Class<*>, methodName: String, argTypes: Array<Class<*>>): Method? {
        return try {
            clazz.getMethod(methodName, *argTypes)
        } catch (e: NoSuchMethodException) {
            for (method in clazz.methods) {
                if (method.name == methodName && method.parameterTypes.size == argTypes.size) {
                    var match = true
                    for (i in argTypes.indices) {
                        if (!method.parameterTypes[i].isAssignableFrom(argTypes[i]) &&
                            !isPrimitiveAssignable(method.parameterTypes[i], argTypes[i])) {
                            match = false
                            break
                        }
                    }
                    if (match) return method
                }
            }
            null
        }
    }
    
    private fun isPrimitiveAssignable(target: Class<*>, source: Class<*>): Boolean {
        if (target == source) return true
        if (target == java.lang.Integer.TYPE && source == java.lang.Integer::class.java) return true
        if (target == java.lang.Long.TYPE && source == java.lang.Long::class.java) return true
        if (target == java.lang.Float.TYPE && source == java.lang.Float::class.java) return true
        if (target == java.lang.Double.TYPE && source == java.lang.Double::class.java) return true
        if (target == java.lang.Boolean.TYPE && source == java.lang.Boolean::class.java) return true
        return false
    }
    
    fun hasMethod(obj: Any, methodName: String): Boolean {
        return obj::class.java.methods.any { it.name == methodName }
    }
    
    fun getAllMethods(obj: Any): List<String> {
        return obj::class.java.declaredMethods.filter { !Modifier.isStatic(it.modifiers) }.map { it.name }
    }
    
    @JvmStatic
    fun invokeStaticMethod(className: String, methodName: String, vararg args: Any): Any? {
        return try {
            val clazz = Class.forName(className)
            val argTypes = args.map { it::class.java }.toTypedArray()
            val method = clazz.getMethod(methodName, *argTypes)
            method.invoke(null, *args)
        } catch (e: Exception) {
            null
        }
    }
    
    @JvmStatic
    fun getStaticField(className: String, fieldName: String): Any? {
        return try {
            val clazz = Class.forName(className)
            val field = clazz.getField(fieldName)
            field.get(null)
        } catch (e: Exception) {
            null
        }
    }
    
    @JvmStatic
    fun setStaticField(className: String, fieldName: String, value: Any) {
        try {
            val clazz = Class.forName(className)
            val field = clazz.getField(fieldName)
            field.set(null, value)
        } catch (e: Exception) {
        }
    }
    
    @JvmStatic
    fun toLuaTable(obj: Any): Map<String, Any> {
        val result = mutableMapOf<String, Any>()
        try {
            for (field in obj::class.java.declaredFields) {
                if (!Modifier.isStatic(field.modifiers)) {
                    field.isAccessible = true
                    val value = field.get(obj)
                    result[field.name] = when {
                        value == null -> "null"
                        isPrimitiveType(value::class.java) -> value
                        value is Array<*> -> value.toList()
                        value is Collection<*> -> value.toList()
                        value is Map<*, *> -> value.entries.associate { it.key.toString() to it.value }
                        else -> value
                    }
                }
            }
        } catch (e: Exception) {
        }
        return result
    }
    
    @JvmStatic
    fun fromLuaTable(table: Map<String, Any>): Any {
        return SimpleDataClass(table)
    }
    
    @JvmStatic
    fun fromLuaTableWithType(table: Map<String, Any>, className: String): Any {
        return SimpleDataClass(table)
    }
    
    private fun isPrimitiveType(clazz: Class<*>): Boolean {
        return clazz.isPrimitive() || 
               clazz == java.lang.Integer::class.java ||
               clazz == java.lang.Long::class.java ||
               clazz == java.lang.Float::class.java ||
               clazz == java.lang.Double::class.java ||
               clazz == java.lang.Boolean::class.java ||
               clazz == java.lang.String::class.java ||
               clazz == java.lang.Byte::class.java ||
               clazz == java.lang.Short::class.java ||
               clazz == java.lang.Character::class.java
    }
    
    @JvmStatic
    fun clone(obj: Any): Any {
        val fields = toLuaTable(obj)
        return SimpleDataClass(fields)
    }
    
    @JvmStatic
    fun deepClone(obj: Any): Any {
        val fields = deepCopyFields(obj)
        return SimpleDataClass(fields)
    }
    
    private fun deepCopyFields(obj: Any): Map<String, Any> {
        val result = mutableMapOf<String, Any>()
        try {
            for (field in obj::class.java.declaredFields) {
                if (!Modifier.isStatic(field.modifiers)) {
                    field.isAccessible = true
                    val value = field.get(obj)
                    result[field.name] = when {
                        value == null -> "null"
                        isPrimitiveType(value::class.java) -> value
                        value is Array<*> -> value.map { deepCopyIfNeeded(it) }.toTypedArray()
                        value is Collection<*> -> value.map { deepCopyIfNeeded(it) }
                        value is Map<*, *> -> value.entries.associate { 
                            (deepCopyIfNeeded(it.key) as String) to deepCopyIfNeeded(it.value) 
                        }
                        else -> toLuaTable(value)
                    }
                }
            }
        } catch (e: Exception) {
        }
        return result
    }
    
    private fun deepCopyIfNeeded(value: Any?): Any {
        return when (value) {
            null -> "null"
            is PrimitiveWrapper -> (value as PrimitiveWrapper).value
            is Array<*> -> value.map { deepCopyIfNeeded(it) }.toTypedArray()
            is Collection<*> -> value.map { deepCopyIfNeeded(it) }
            else -> clone(value)
        }
    }
    
    class PrimitiveWrapper(val value: Any)
    
    @JvmStatic
    fun equals(obj1: Any, obj2: Any): Boolean {
        if (obj1::class.java != obj2::class.java) return false
        return toLuaTable(obj1) == toLuaTable(obj2)
    }
    
    @JvmStatic
    fun toString(obj: Any): String {
        val sb = StringBuilder()
        sb.append(obj::class.java.simpleName).append(" { ")
        val fields = toLuaTable(obj)
        var index = 0
        for ((key, value) in fields) {
            sb.append("$key=$value")
            if (index < fields.size - 1) sb.append(", ")
            index++
        }
        sb.append(" }")
        return sb.toString()
    }
    
    @JvmStatic
    fun hashCode(obj: Any): Int {
        return toLuaTable(obj).hashCode()
    }
    
    @JvmStatic
    fun typeOf(obj: Any): String = obj::class.java.typeName
    
    @JvmStatic
    fun typeName(obj: Any): String = obj::class.java.simpleName
    
    @JvmStatic
    fun isArray(obj: Any): Boolean = obj.javaClass.isArray
    
    @JvmStatic
    fun isList(obj: Any): Boolean = obj is List<*>
    
    @JvmStatic
    fun isMap(obj: Any): Boolean = obj is Map<*, *>
    
    @JvmStatic
    fun isNumber(obj: Any): Boolean = obj is Number
    
    @JvmStatic
    fun isString(obj: Any): Boolean = obj is String
    
    @JvmStatic
    fun isBoolean(obj: Any): Boolean = obj is Boolean
    
    @JvmStatic
    fun asString(obj: Any): String = obj.toString()
    
    @JvmStatic
    fun asInt(obj: Any): Int = (obj as? Number)?.toInt() ?: 0
    
    @JvmStatic
    fun asLong(obj: Any): Long = (obj as? Number)?.toLong() ?: 0L
    
    @JvmStatic
    fun asFloat(obj: Any): Float = (obj as? Number)?.toFloat() ?: 0f
    
    @JvmStatic
    fun asDouble(obj: Any): Double = (obj as? Number)?.toDouble() ?: 0.0
    
    @JvmStatic
    fun asBoolean(obj: Any): Boolean = obj as? Boolean ?: false
    
    @JvmStatic
    fun convertType(value: Any, targetType: String): Any {
        return when (targetType) {
            "int", "java.lang.Integer" -> asInt(value)
            "long", "java.lang.Long" -> asLong(value)
            "float", "java.lang.Float" -> asFloat(value)
            "double", "java.lang.Double" -> asDouble(value)
            "boolean", "java.lang.Boolean" -> asBoolean(value)
            "java.lang.String" -> asString(value)
            else -> value
        }
    }
    
    private fun convertValue(value: Any, targetType: Class<*>): Any {
        return when {
            targetType == Integer.TYPE && value is Number -> value.toInt()
            targetType == java.lang.Long.TYPE && value is Number -> value.toLong()
            targetType == java.lang.Float.TYPE && value is Number -> value.toFloat()
            targetType == java.lang.Double.TYPE && value is Number -> value.toDouble()
            targetType == java.lang.Boolean.TYPE && value is Boolean -> value
            targetType == String::class.java && value is String -> value
            targetType == Integer::class.java && value is Number -> value.toInt()
            targetType == java.lang.Long::class.java && value is Number -> value.toLong()
            targetType == java.lang.Float::class.java && value is Number -> value.toFloat()
            targetType == java.lang.Double::class.java && value is Number -> value.toDouble()
            targetType == java.lang.Boolean::class.java && value is Boolean -> value
            else -> value
        }
    }
    
    @JvmStatic
    fun createProxy(
        interfaceName: String,
        handlers: Map<String, Any>
    ): Any {
        return try {
            val interfaceClass = Class.forName(interfaceName)
            java.lang.reflect.Proxy.newProxyInstance(
                interfaceClass.classLoader,
                arrayOf(interfaceClass),
                InvocationHandler { _, method, args ->
                    val methodName = method.name
                    val handler = handlers[methodName] ?: handlers["*"]
                    null
                }
            )
        } catch (e: Exception) {
            SimpleDataClass(emptyMap())
        }
    }
    
    @JvmStatic
    fun createDynamicProxy(
        interfaceNames: List<String>,
        invokeHandler: Any
    ): Any {
        return try {
            val interfaces = interfaceNames.map { Class.forName(it) }.toTypedArray()
            java.lang.reflect.Proxy.newProxyInstance(
                interfaces.first().classLoader,
                interfaces
            ) { _, method, args ->
                null
            }
        } catch (e: Exception) {
            SimpleDataClass(emptyMap())
        }
    }
    
    @JvmStatic
    fun addPropertyChangeListener(obj: Any, listener: (String, Any?, Any?) -> Unit): String {
        val id = java.util.UUID.randomUUID().toString()
        PropertyChangeSupport(obj).addListener(id, listener)
        return id
    }
    
    @JvmStatic
    fun removePropertyChangeListener(obj: Any, listenerId: String) {
        PropertyChangeSupport(obj).removeListener(listenerId)
    }
    
    @JvmStatic
    fun firePropertyChange(obj: Any, propertyName: String, oldValue: Any?, newValue: Any?) {
        PropertyChangeSupport(obj).fireChange(propertyName, oldValue, newValue)
    }
    
    private class PropertyChangeSupport(private val obj: Any) {
        private val listeners = ConcurrentHashMap<String, (String, Any?, Any?) -> Unit>()
        
        fun addListener(id: String, listener: (String, Any?, Any?) -> Unit) {
            listeners[id] = listener
        }
        
        fun removeListener(id: String) {
            listeners.remove(id)
        }
        
        fun fireChange(propertyName: String, oldValue: Any?, newValue: Any?) {
            listeners.values.forEach { it(propertyName, oldValue, newValue) }
        }
    }
    
    @JvmStatic
    fun createObservable(obj: Any): Observable {
        return Observable(obj)
    }
    
    class Observable(private val obj: Any) {
        private val listeners = CopyOnWriteArrayList<() -> Unit>()
        private val propertyListeners = CopyOnWriteArrayList<(String, Any?, Any?) -> Unit>()
        
        fun addListener(callback: () -> Unit): Int {
            val id = listeners.size
            listeners.add(callback)
            return id
        }
        
        fun removeListener(id: Int) {
            if (id < listeners.size) listeners.removeAt(id)
        }
        
        fun addPropertyListener(callback: (String, Any?, Any?) -> Unit): Int {
            val id = propertyListeners.size
            propertyListeners.add(callback)
            return id
        }
        
        fun removePropertyListener(id: Int) {
            if (id < propertyListeners.size) propertyListeners.removeAt(id)
        }
        
        fun notifyChange() {
            listeners.forEach { it() }
        }
        
        fun notifyPropertyChange(propertyName: String, oldValue: Any?, newValue: Any?) {
            propertyListeners.forEach { it(propertyName, oldValue, newValue) }
        }
        
        fun getTarget(): Any = obj
    }
    
    @JvmStatic
    fun clearCache() {
        classCache.clear()
        methodCache.clear()
        constructorCache.clear()
    }
    
    @JvmStatic
    fun getCacheSize(): Int = classCache.size + methodCache.size + constructorCache.size
}

class SimpleDataClass(private val data: Map<String, Any>) {
    
    operator fun get(key: String): Any? = data[key]
    
    operator fun set(key: String, value: Any): SimpleDataClass {
        val newData = data.toMutableMap()
        newData[key] = value
        return SimpleDataClass(newData)
    }
    
    fun getOrDefault(key: String, default: Any): Any = data[key] ?: default
    
    fun contains(key: String): Boolean = data.containsKey(key)
    
    fun keys(): Set<String> = data.keys
    
    fun values(): Collection<Any> = data.values
    
    fun entries(): Set<Map.Entry<String, Any>> = data.entries
    
    fun size(): Int = data.size
    
    fun isEmpty(): Boolean = data.isEmpty()
    
    fun toMap(): Map<String, Any> = data.toMap()
    
    fun toJson(): String {
        val sb = StringBuilder()
        sb.append("{ ")
        var index = 0
        for ((key, value) in data) {
            sb.append("\"$key\":")
            when (value) {
                is String -> sb.append("\"$value\"")
                is Number -> sb.append(value)
                is Boolean -> sb.append(value)
                is SimpleDataClass -> sb.append(value.toJson())
                is List<*> -> sb.append(listToJson(value))
                is Map<*, *> -> sb.append(mapToJson(value))
                else -> sb.append("\"$value\"")
            }
            if (index < data.size - 1) sb.append(", ")
            index++
        }
        sb.append(" }")
        return sb.toString()
    }
    
    private fun listToJson(list: List<*>): String {
        val sb = StringBuilder()
        sb.append("[")
        for (i in list.indices) {
            val item = list[i]
            when (item) {
                is String -> sb.append("\"$item\"")
                is Number -> sb.append(item)
                is Boolean -> sb.append(item)
                is SimpleDataClass -> sb.append(item.toJson())
                is List<*> -> sb.append(listToJson(item))
                is Map<*, *> -> sb.append(mapToJson(item))
                null -> sb.append("null")
                else -> sb.append("\"$item\"")
            }
            if (i < list.size - 1) sb.append(", ")
        }
        sb.append("]")
        return sb.toString()
    }
    
    private fun mapToJson(map: Map<*, *>): String {
        val sb = StringBuilder()
        sb.append("{ ")
        val entries = map.entries.toList()
        for (i in entries.indices) {
            val entry = entries[i]
            sb.append("\"${entry.key}\":")
            val value = entry.value
            when (value) {
                is String -> sb.append("\"$value\"")
                is Number -> sb.append(value)
                is Boolean -> sb.append(value)
                is SimpleDataClass -> sb.append(value.toJson())
                is List<*> -> sb.append(listToJson(value))
                is Map<*, *> -> sb.append(mapToJson(value))
                null -> sb.append("null")
                else -> sb.append("\"$value\"")
            }
            if (i < entries.size - 1) sb.append(", ")
        }
        sb.append(" }")
        return sb.toString()
    }
    
    fun merge(other: SimpleDataClass): SimpleDataClass {
        return SimpleDataClass(data + other.data)
    }
    
    fun remove(key: String): SimpleDataClass {
        val newData = data.toMutableMap()
        newData.remove(key)
        return SimpleDataClass(newData)
    }
    
    fun filter(predicate: (String, Any) -> Boolean): SimpleDataClass {
        return SimpleDataClass(data.filter { predicate(it.key, it.value) })
    }
    
    fun map(transform: (String, Any) -> Pair<String, Any>): SimpleDataClass {
        return SimpleDataClass(data.map { transform(it.key, it.value) }.toMap())
    }
    
    fun forEach(action: (String, Any) -> Unit) {
        data.forEach { action(it.key, it.value) }
    }
    
    fun getInt(key: String): Int = (data[key] as? Number)?.toInt() ?: 0
    
    fun getLong(key: String): Long = (data[key] as? Number)?.toLong() ?: 0L
    
    fun getFloat(key: String): Float = (data[key] as? Number)?.toFloat() ?: 0f
    
    fun getDouble(key: String): Double = (data[key] as? Number)?.toDouble() ?: 0.0
    
    fun getBoolean(key: String): Boolean = data[key] as? Boolean ?: false
    
    fun getString(key: String): String = data[key]?.toString() ?: ""
    
    fun getList(key: String): List<Any> = (data[key] as? List<*>)?.filterIsInstance<Any>() ?: emptyList()
    
    fun getMap(key: String): Map<String, Any> = (data[key] as? Map<*, *>)?.entries?.associate { 
        it.key.toString() to it.value!! 
    } ?: emptyMap()
    
    fun getSimpleDataClass(key: String): SimpleDataClass? {
        return data[key] as? SimpleDataClass
    }
    
    fun setInt(key: String, value: Int): SimpleDataClass = set(key, value)
    fun setLong(key: String, value: Long): SimpleDataClass = set(key, value)
    fun setFloat(key: String, value: Float): SimpleDataClass = set(key, value)
    fun setDouble(key: String, value: Double): SimpleDataClass = set(key, value)
    fun setBoolean(key: String, value: Boolean): SimpleDataClass = set(key, value)
    fun setString(key: String, value: String): SimpleDataClass = set(key, value)
    fun setList(key: String, value: List<Any>): SimpleDataClass = set(key, value)
    fun setMap(key: String, value: Map<String, Any>): SimpleDataClass = set(key, value)
    
    override fun toString(): String = toJson()
    
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (other !is SimpleDataClass) return false
        return data == other.data
    }
    
    override fun hashCode(): Int = data.hashCode()
}

object LuaArrayUtils {
    
    fun newByteArray(size: Int): ByteArray = ByteArray(size)
    fun newShortArray(size: Int): ShortArray = ShortArray(size)
    fun newIntArray(size: Int): IntArray = IntArray(size)
    fun newLongArray(size: Int): LongArray = LongArray(size)
    fun newFloatArray(size: Int): FloatArray = FloatArray(size)
    fun newDoubleArray(size: Int): DoubleArray = DoubleArray(size)
    fun newBooleanArray(size: Int): BooleanArray = BooleanArray(size)
    fun newCharArray(size: Int): CharArray = CharArray(size)
    fun newObjectArray(size: Int): Array<Any?> = arrayOfNulls(size)
    fun newStringArray(size: Int): Array<String> = Array(size) { "" }
    
    fun setByte(arr: ByteArray, index: Int, value: Byte) { arr[index] = value }
    fun setShort(arr: ShortArray, index: Int, value: Short) { arr[index] = value }
    fun setInt(arr: IntArray, index: Int, value: Int) { arr[index] = value }
    fun setLong(arr: LongArray, index: Int, value: Long) { arr[index] = value }
    fun setFloat(arr: FloatArray, index: Int, value: Float) { arr[index] = value }
    fun setDouble(arr: DoubleArray, index: Int, value: Double) { arr[index] = value }
    fun setBoolean(arr: BooleanArray, index: Int, value: Boolean) { arr[index] = value }
    fun setChar(arr: CharArray, index: Int, value: Char) { arr[index] = value }
    fun setObject(arr: Array<Any?>, index: Int, value: Any?) { arr[index] = value }
    fun setString(arr: Array<String>, index: Int, value: String) { arr[index] = value }
    
    fun getByte(arr: ByteArray, index: Int): Byte = arr[index]
    fun getShort(arr: ShortArray, index: Int): Short = arr[index]
    fun getInt(arr: IntArray, index: Int): Int = arr[index]
    fun getLong(arr: LongArray, index: Int): Long = arr[index]
    fun getFloat(arr: FloatArray, index: Int): Float = arr[index]
    fun getDouble(arr: DoubleArray, index: Int): Double = arr[index]
    fun getBoolean(arr: BooleanArray, index: Int): Boolean = arr[index]
    fun getChar(arr: CharArray, index: Int): Char = arr[index]
    fun getObject(arr: Array<Any?>, index: Int): Any? = arr[index]
    fun getString(arr: Array<String>, index: Int): String = arr[index]
    
    fun getLength(arr: Any): Int {
        return when (arr) {
            is ByteArray -> arr.size
            is ShortArray -> arr.size
            is IntArray -> arr.size
            is LongArray -> arr.size
            is FloatArray -> arr.size
            is DoubleArray -> arr.size
            is BooleanArray -> arr.size
            is CharArray -> arr.size
            is Array<*> -> arr.size
            else -> 0
        }
    }
    
    fun toList(arr: Any): List<Any> {
        return when (arr) {
            is ByteArray -> arr.toList()
            is ShortArray -> arr.toList()
            is IntArray -> arr.toList()
            is LongArray -> arr.toList()
            is FloatArray -> arr.toList()
            is DoubleArray -> arr.toList()
            is BooleanArray -> arr.toList()
            is CharArray -> arr.toList()
            is Array<*> -> arr.filterNotNull()
            else -> emptyList<Any>()
        }
    }
    
    fun copyOf(arr: Any): Any {
        return when (arr) {
            is ByteArray -> arr.copyOf()
            is ShortArray -> arr.copyOf()
            is IntArray -> arr.copyOf()
            is LongArray -> arr.copyOf()
            is FloatArray -> arr.copyOf()
            is DoubleArray -> arr.copyOf()
            is BooleanArray -> arr.copyOf()
            is CharArray -> arr.copyOf()
            is Array<*> -> arr.copyOf()
            else -> arr
        }
    }
    
    fun fill(arr: Any, value: Any) {
        when (arr) {
            is ByteArray -> { if (value is Byte) arr.fill(value) }
            is ShortArray -> { if (value is Short) arr.fill(value) }
            is IntArray -> { if (value is Int) arr.fill(value) }
            is LongArray -> { if (value is Long) arr.fill(value) }
            is FloatArray -> { if (value is Float) arr.fill(value) }
            is DoubleArray -> { if (value is Double) arr.fill(value) }
            is BooleanArray -> { if (value is Boolean) arr.fill(value) }
            is CharArray -> { if (value is Char) arr.fill(value) }
        }
    }
    
    fun sortInt(arr: IntArray) = arr.sort()
    fun sortLong(arr: LongArray) = arr.sort()
    fun sortFloat(arr: FloatArray) = arr.sort()
    fun sortDouble(arr: DoubleArray) = arr.sort()
}

object LuaCollectionUtils {
    
    fun newArrayList(): java.util.ArrayList<Any> = java.util.ArrayList()
    fun newLinkedList(): java.util.LinkedList<Any> = java.util.LinkedList()
    fun newVector(): java.util.Vector<Any> = java.util.Vector()
    fun newStack(): java.util.Stack<Any> = java.util.Stack()
    
    fun newHashSet(): java.util.HashSet<Any> = java.util.HashSet()
    fun newLinkedHashSet(): java.util.LinkedHashSet<Any> = java.util.LinkedHashSet()
    fun newTreeSet(): java.util.TreeSet<Any> = java.util.TreeSet()
    
    fun newHashMap(): java.util.HashMap<Any, Any> = java.util.HashMap()
    fun newLinkedHashMap(): java.util.LinkedHashMap<Any, Any> = java.util.LinkedHashMap()
    fun newTreeMap(): java.util.TreeMap<Any, Any> = java.util.TreeMap()
    fun newHashtable(): java.util.Hashtable<Any, Any> = java.util.Hashtable()
    fun newProperties(): java.util.Properties = java.util.Properties()
    
    fun <T> add(list: java.util.ArrayList<T>, item: T): Boolean = list.add(item)
    fun <T> addFirst(list: java.util.LinkedList<T>, item: T) { list.addFirst(item) }
    fun <T> addLast(list: java.util.LinkedList<T>, item: T) { list.addLast(item) }
    fun <T> push(stack: java.util.Stack<T>, item: T) { stack.push(item) }
    
    fun <K, V> put(map: java.util.HashMap<K, V>, key: K, value: V): V? = map.put(key, value)
    
    fun <T> get(list: java.util.ArrayList<T>, index: Int): T? = if (index >= 0 && index < list.size) list[index] else null
    fun <T> getFirst(list: java.util.LinkedList<T>): T? = list.peekFirst()
    fun <T> getLast(list: java.util.LinkedList<T>): T? = list.peekLast()
    fun <T> pop(stack: java.util.Stack<T>): T? = if (stack.isEmpty()) null else stack.pop()
    fun <T> peek(stack: java.util.Stack<T>): T? = if (stack.isEmpty()) null else stack.peek()
    fun <K, V> get(map: java.util.HashMap<K, V>, key: K): V? = map[key]
    
    fun <T> contains(list: java.util.ArrayList<T>, item: T): Boolean = list.contains(item)
    fun <T> contains(set: java.util.HashSet<T>, item: T): Boolean = set.contains(item)
    fun <K, V> containsKey(map: java.util.HashMap<K, V>, key: K): Boolean = map.containsKey(key)
    fun <K, V> containsValue(map: java.util.HashMap<K, V>, value: V): Boolean = map.containsValue(value)
    
    fun <T> remove(list: java.util.ArrayList<T>, item: T): Boolean = list.remove(item)
    fun <T> removeAt(list: java.util.ArrayList<T>, index: Int): T? = if (index >= 0 && index < list.size) list.removeAt(index) else null
    fun <T> removeFirst(list: java.util.LinkedList<T>): T? = list.pollFirst()
    fun <T> removeLast(list: java.util.LinkedList<T>): T? = list.pollLast()
    fun <K, V> remove(map: java.util.HashMap<K, V>, key: K): V? = map.remove(key)
    
    fun <T> clear(list: java.util.ArrayList<T>) { list.clear() }
    fun <T> clear(set: java.util.HashSet<T>) { set.clear() }
    fun <K, V> clear(map: java.util.HashMap<K, V>) { map.clear() }
    
    fun <T> size(list: java.util.ArrayList<T>): Int = list.size
    fun <T> size(set: java.util.HashSet<T>): Int = set.size
    fun <K, V> size(map: java.util.HashMap<K, V>): Int = map.size
    
    fun <T> isEmpty(list: java.util.ArrayList<T>): Boolean = list.isEmpty()
    fun <T> isEmpty(set: java.util.HashSet<T>): Boolean = set.isEmpty()
    fun <K, V> isEmpty(map: java.util.HashMap<K, V>): Boolean = map.isEmpty()
    
    fun <T> shuffle(list: java.util.ArrayList<T>) { list.shuffle() }
    fun <T> reverse(list: java.util.ArrayList<T>) { list.reverse() }
}

object LuaTypeUtils {
    
    fun getTypeName(obj: Any): String = obj::class.java.simpleName
    
    fun getFullTypeName(obj: Any): String = obj::class.java.name
    
    fun isType(obj: Any, className: String): Boolean {
        return try {
            val targetClass = Class.forName(className)
            targetClass.isInstance(obj)
        } catch (e: Exception) {
            false
        }
    }
    
    fun isInt(obj: Any): Boolean = obj is Int
    fun isLong(obj: Any): Boolean = obj is Long
    fun isFloat(obj: Any): Boolean = obj is Float
    fun isDouble(obj: Any): Boolean = obj is Double
    fun isNumber(obj: Any): Boolean = obj is Number
    fun isBoolean(obj: Any): Boolean = obj is Boolean
    fun isString(obj: Any): Boolean = obj is String
    fun isArray(obj: Any): Boolean = obj.javaClass.isArray
    fun isList(obj: Any): Boolean = obj is List<*>
    fun isMap(obj: Any): Boolean = obj is Map<*, *>
    fun isCollection(obj: Any): Boolean = obj is Collection<*>
    
    fun isPrimitive(obj: Any): Boolean {
        return obj is Int || obj is Long || obj is Float || 
               obj is Double || obj is Boolean || obj is Char ||
               obj is Short || obj is Byte
    }
    
    fun isNumeric(obj: Any): Boolean {
        return when (obj) {
            is Number -> true
            is String -> obj.toDoubleOrNull() != null
            else -> false
        }
    }
    
    fun toInt(obj: Any): Int = (obj as? Number)?.toInt() ?: 0
    fun toLong(obj: Any): Long = (obj as? Number)?.toLong() ?: 0L
    fun toFloat(obj: Any): Float = (obj as? Number)?.toFloat() ?: 0f
    fun toDouble(obj: Any): Double = (obj as? Number)?.toDouble() ?: 0.0
    fun toBoolean(obj: Any): Boolean = obj as? Boolean ?: false
    fun toString(obj: Any): String = obj.toString()
    
    fun safeCast(obj: Any, targetType: Class<*>): Any? {
        return if (targetType.isInstance(obj)) obj else null
    }
    
    fun getComponentType(arr: Any): String? {
        return try {
            val componentType = arr.javaClass.componentType
            componentType?.simpleName
        } catch (e: Exception) {
            null
        }
    }
    
    fun getInterfaces(obj: Any): List<String> {
        return obj::class.java.interfaces.map { it.simpleName }
    }
    
    fun getSuperclass(obj: Any): String? {
        val superclass = obj::class.java.superclass
        return if (superclass != null) superclass.simpleName else null
    }
    
    fun getAllSuperclasses(obj: Any): List<String> {
        val result = mutableListOf<String>()
        var superclass = obj::class.java.superclass
        while (superclass != null && superclass != Any::class.java) {
            result.add(superclass.simpleName)
            superclass = superclass.superclass
        }
        return result
    }
    
    fun isAssignableFrom(parent: String, child: String): Boolean {
        return try {
            val parentClass = Class.forName(parent)
            val childClass = Class.forName(child)
            parentClass.isAssignableFrom(childClass)
        } catch (e: Exception) {
            false
        }
    }
}

object LuaMathUtils {
    
    fun clamp(value: Int, min: Int, max: Int): Int = value.coerceIn(min, max)
    fun clamp(value: Long, min: Long, max: Long): Long = value.coerceIn(min, max)
    fun clamp(value: Float, min: Float, max: Float): Float = value.coerceIn(min, max)
    fun clamp(value: Double, min: Double, max: Double): Double = value.coerceIn(min, max)
    
    fun lerp(start: Float, end: Float, t: Float): Float {
        return start + (end - start) * t.coerceIn(0f, 1f)
    }
    
    fun lerp(start: Double, end: Double, t: Double): Double {
        return start + (end - start) * t.coerceIn(0.0, 1.0)
    }
    
    fun map(value: Float, inMin: Float, inMax: Float, outMin: Float, outMax: Float): Float {
        return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin
    }
    
    fun distance(x1: Float, y1: Float, x2: Float, y2: Float): Float {
        val dx = x2 - x1
        val dy = y2 - y1
        return sqrt(dx * dx + dy * dy)
    }
    
    fun distance3D(x1: Float, y1: Float, z1: Float, x2: Float, y2: Float, z2: Float): Float {
        val dx = x2 - x1
        val dy = y2 - y1
        val dz = z2 - z1
        return sqrt(dx * dx + dy * dy + dz * dz)
    }
    
    fun angle(x1: Float, y1: Float, x2: Float, y2: Float): Float {
        return Math.toDegrees(atan2((y2 - y1).toDouble(), (x2 - x1).toDouble())).toFloat()
    }
    
    fun radians(degrees: Float): Float = Math.toRadians(degrees.toDouble()).toFloat()
    fun degrees(radians: Float): Float = Math.toDegrees(radians.toDouble()).toFloat()
    
    fun randomInt(min: Int, max: Int): Int {
        return (min + Math.random() * (max - min + 1)).toInt()
    }
    
    fun randomFloat(min: Float, max: Float): Float {
        return min + (Math.random() * (max - min)).toFloat()
    }
    
    fun randomDouble(min: Double, max: Double): Double {
        return min + Math.random() * (max - min)
    }
    
    fun randomBool(): Boolean = Math.random() < 0.5
    fun randomSign(): Int = if (Math.random() < 0.5) -1 else 1
    
    fun abs(value: Int): Int = abs(value)
    fun abs(value: Long): Long = abs(value)
    fun abs(value: Float): Float = abs(value)
    fun abs(value: Double): Double = abs(value)
    
    fun floor(value: Double): Double = floor(value)
    fun ceil(value: Double): Double = ceil(value)
    fun round(value: Double): Double = kotlin.math.round(value)
    
    fun sqrt(value: Double): Double = sqrt(value)
    fun pow(base: Double, exp: Double): Double = base.pow(exp)
    
    fun sin(angle: Float): Float = sin(Math.toRadians(angle.toDouble())).toFloat()
    fun cos(angle: Float): Float = cos(Math.toRadians(angle.toDouble())).toFloat()
    fun tan(angle: Float): Float = tan(Math.toRadians(angle.toDouble())).toFloat()
    
    fun asin(value: Float): Float = asin(value.toDouble()).toFloat()
    fun acos(value: Float): Float = acos(value.toDouble()).toFloat()
    fun atan(value: Float): Float = atan(value.toDouble()).toFloat()
    fun atan2(y: Float, x: Float): Float = atan2(y.toDouble(), x.toDouble()).toFloat()
}
