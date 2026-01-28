package com.difierline.lua;

import com.luajava.LuaState;
import com.luajava.LuaObject;
import com.luajava.JavaFunction;
import com.luajava.LuaException;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.ScriptableObject;
import org.mozilla.javascript.ImporterTopLevel;
import org.mozilla.javascript.Function;
import org.mozilla.javascript.EcmaError;
import org.mozilla.javascript.NativeJavaObject;
import org.mozilla.javascript.NativeJavaClass;
import org.mozilla.javascript.Undefined;
import org.mozilla.javascript.NativeArray;
import org.mozilla.javascript.NativeObject;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

/**
 * JavaScript 引擎包装类，用于将 Rhino 引擎的功能暴露给 Lua
 * 提供 Lua 与 JavaScript 之间的双向互操作
 */
public class JavaScriptEngine {
    private Context context;
    private Scriptable scope;
    private LuaContext luaContext;
    private boolean sandboxMode;
    private Set<String> allowedPackages;

    /**
     * 构造函数
     * @param luaContext Lua 上下文
     */
    public JavaScriptEngine(LuaContext luaContext) {
        this.luaContext = luaContext;
        this.sandboxMode = false;
        this.allowedPackages = new HashSet<>();
        init();
    }

    /**
     * 构造函数
     * @param luaContext Lua 上下文
     * @param sandboxMode 是否启用沙箱模式
     */
    public JavaScriptEngine(LuaContext luaContext, boolean sandboxMode) {
        this.luaContext = luaContext;
        this.sandboxMode = sandboxMode;
        this.allowedPackages = new HashSet<>();
        init();
    }

    /**
     * 初始化 JavaScript 引擎
     */
    private void init() {
        context = Context.enter();
        context.setLanguageVersion(Context.VERSION_ES6);
        context.setOptimizationLevel(-1);
        scope = new ImporterTopLevel(context);
        
        enableJavaAccess();
        registerLuaBridge();
        
        // 暴露 Android Context 到 JavaScript 作用域
        if (luaContext != null) {
            android.content.Context androidContext = luaContext.getContext();
            if (androidContext != null) {
                ScriptableObject.putProperty(scope, "context", Context.javaToJS(androidContext, scope));
            }
        }
    }

    /**
     * 注册 Lua 桥接对象到 JavaScript 作用域
     */
    private void registerLuaBridge() {
        Scriptable luaBridge = context.newObject(scope);
        ScriptableObject.putProperty(scope, "lua", luaBridge);

        ScriptableObject.putProperty(luaBridge, "call",
            new org.mozilla.javascript.BaseFunction() {
                @Override
                public Object call(org.mozilla.javascript.Context cx, Scriptable scope,
                                  Scriptable thisObj, Object[] args) {
                    if (args.length < 1 || !(args[0] instanceof String)) {
                        throw new RuntimeException("lua.call 需要函数名作为第一个参数");
                    }
                    String funcName = (String) args[0];
                    List<Object> javaArgs = new ArrayList<>();
                    for (int i = 1; i < args.length; i++) {
                        javaArgs.add(args[i]);
                    }
                    Object result = JavaScriptEngine.this.callLua(funcName, javaArgs);
                    // 将返回值转换为JavaScript可处理的类型
                    return Context.javaToJS(result, scope);
                }
            });

        ScriptableObject.putProperty(luaBridge, "eval",
            new org.mozilla.javascript.BaseFunction() {
                @Override
                public Object call(org.mozilla.javascript.Context cx, Scriptable scope,
                                  Scriptable thisObj, Object[] args) {
                    if (args.length < 1 || !(args[0] instanceof String)) {
                        throw new RuntimeException("lua.eval 需要代码字符串作为参数");
                    }
                    Object result = JavaScriptEngine.this.evalLua((String) args[0]);
                    // 将返回值转换为JavaScript可处理的类型
                    return Context.javaToJS(result, scope);
                }
            });
            
        // 添加序列化方法
        ScriptableObject.putProperty(luaBridge, "serialize",
            new org.mozilla.javascript.BaseFunction() {
                @Override
                public Object call(org.mozilla.javascript.Context cx, Scriptable scope,
                                  Scriptable thisObj, Object[] args) {
                    if (args.length < 1) {
                        throw new RuntimeException("lua.serialize 需要一个参数");
                    }
                    Object luaValue = args[0];
                    return JavaScriptEngine.this.serialize(luaValue);
                }
            });
            
        // 添加转换为JavaScript对象的方法
        ScriptableObject.putProperty(luaBridge, "toJSObject",
            new org.mozilla.javascript.BaseFunction() {
                @Override
                public Object call(org.mozilla.javascript.Context cx, Scriptable scope,
                                  Scriptable thisObj, Object[] args) {
                    if (args.length < 1) {
                        throw new RuntimeException("lua.toJSObject 需要一个参数");
                    }
                    Object luaValue = args[0];
                    return JavaScriptEngine.this.toJSObject(luaValue, scope);
                }
            });
            
        // 添加注册JavaScript函数到Lua的方法
        ScriptableObject.putProperty(luaBridge, "register",
            new org.mozilla.javascript.BaseFunction() {
                @Override
                public Object call(org.mozilla.javascript.Context cx, Scriptable scope,
                                  Scriptable thisObj, Object[] args) {
                    if (args.length < 2 || !(args[0] instanceof String)) {
                        throw new RuntimeException("lua.register 需要函数名和函数对象作为参数");
                    }
                    String funcName = (String) args[0];
                    Object jsFunction = args[1];
                    
                    // 调用bind方法将JavaScript函数转换为Lua可调用对象
                    Object luaFunction = JavaScriptEngine.this.bind(jsFunction);
                    
                    // 将转换后的Lua函数注册到全局表
                    LuaState L = luaContext.getLuaState();
                    if (luaFunction instanceof JavaFunction) {
                        try {
                            ((JavaFunction) luaFunction).register(funcName);
                        } catch (LuaException e) {
                            throw new RuntimeException("注册Lua函数失败: " + e.getMessage(), e);
                        }
                    }
                    
                    return true;
                }
            });
            
        // 添加在JavaScript中注册Lua函数的方法
        ScriptableObject.putProperty(luaBridge, "bindLua",
            new org.mozilla.javascript.BaseFunction() {
                @Override
                public Object call(org.mozilla.javascript.Context cx, Scriptable scope,
                                  Scriptable thisObj, Object[] args) {
                    if (args.length < 1 || !(args[0] instanceof String)) {
                        throw new RuntimeException("lua.bindLua 需要Lua函数名作为参数");
                    }
                    String funcName = (String) args[0];
                    
                    // 创建一个JavaScript函数，当被调用时会调用对应的Lua函数
                    return new org.mozilla.javascript.BaseFunction() {
                        @Override
                        public Object call(org.mozilla.javascript.Context cx, Scriptable scope,
                                          Scriptable thisObj, Object[] args) {
                            // 将JavaScript参数转换为Java参数列表
                            List<Object> javaArgs = new ArrayList<>();
                            for (Object arg : args) {
                                javaArgs.add(arg);
                            }
                            
                            // 调用Lua函数并返回结果
                            return JavaScriptEngine.this.callLua(funcName, javaArgs);
                        }
                    };
                }
            });
    }



    /**
     * 启用 JavaScript 访问 Java 类的功能
     */
    private void enableJavaAccess() {
        try {
            if (sandboxMode) {
                for (String pkg : allowedPackages) {
                    ScriptableObject.putProperty(scope, pkg, context.newObject(scope));
                }
            } else {
                ScriptableObject.putProperty(scope, "java", context.newObject(scope));
                ScriptableObject.putProperty(scope, "javax", context.newObject(scope));
                ScriptableObject.putProperty(scope, "org", context.newObject(scope));
                ScriptableObject.putProperty(scope, "android", context.newObject(scope));
                ScriptableObject.putProperty(scope, "javautil", context.newObject(scope));
            }
        } catch (Exception e) {
            throw new RuntimeException("启用 Java 访问功能时发生错误: " + e.getMessage(), e);
        }
    }

    /**
     * 设置沙箱模式
     * @param enabled 是否启用沙箱模式
     * @param packages 允许访问的 Java 包列表
     */
    public void setSandbox(boolean enabled, String... packages) {
        this.sandboxMode = enabled;
        this.allowedPackages.clear();
        for (String pkg : packages) {
            allowedPackages.add(pkg);
        }
    }

    /**
     * 执行 JavaScript 代码
     * @param code 要执行的 JavaScript 代码
     * @return 执行结果
     */
    public Object eval(String code) {
        if (code == null) {
            throw new IllegalArgumentException("代码不能为 null");
        }
        try {
            Object result = context.evaluateString(scope, code, "JS", 1, null);
            
            if (result == Undefined.instance) {
                return null;
            }
            
            if (result instanceof Scriptable && !(result instanceof NativeJavaObject)) {
            }
            
            // 确保返回值有效
            if (result == null || result == Undefined.instance) {
                return null;
            }
            
            return Context.jsToJava(result, Object.class);
        } catch (EcmaError e) {
            throw new RuntimeException("JS 执行错误: " + e.getMessage() + " (行 " + e.lineNumber() + ")", e);
        } catch (Exception e) {
            throw new RuntimeException("执行 JS 代码时发生错误: " + e.getMessage(), e);
        }
    }
    
    /**
     * 执行 Lua 代码
     * @param code 要执行的 Lua 代码
     * @return 执行结果
     */
    public Object evalLua(String code) {
        if (code == null) {
            throw new IllegalArgumentException("代码不能为 null");
        }
        try {
            LuaState L = luaContext.getLuaState();
            
            // 保存当前栈顶位置
            int top = L.getTop();
            
            // 加载并执行 Lua 代码
            int result = L.LdoString(code);
            if (result != 0) {
                String error = L.toString(-1);
                L.pop(1);
                throw new RuntimeException("执行 Lua 代码失败: " + error);
            }
            
            // 计算返回值数量（新压入栈的值）
            int returnCount = L.getTop() - top;
            if (returnCount == 0) {
                return null;
            } else if (returnCount == 1) {
                Object value = popLuaValue(L, -1);
                L.pop(1);
                return value;
            } else {
                Object[] results = new Object[returnCount];
                for (int i = 0; i < returnCount; i++) {
                    results[i] = popLuaValue(L, top + i + 1);
                }
                L.pop(returnCount);
                return results;
            }
            
        } catch (RuntimeException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException("执行 Lua 代码时发生错误: " + e.getMessage(), e);
        }
    }

    /**
     * 执行 JavaScript 文件
     * @param filePath JavaScript 文件路径（相对于 Lua 脚本目录）
     * @return 执行结果
     */
    public Object evalFile(String filePath) {
        if (filePath == null) {
            throw new IllegalArgumentException("文件路径不能为 null");
        }
        String fullPath = luaContext.getLuaDir() + "/" + filePath;
        java.io.File file = new java.io.File(fullPath);
        
        try (java.io.FileReader reader = new java.io.FileReader(file)) {
            Object result = context.evaluateReader(scope, reader, filePath, 1, null);
            return Context.jsToJava(result, Object.class);
        } catch (java.io.FileNotFoundException e) {
            throw new RuntimeException("JS 文件未找到: " + filePath, e);
        } catch (java.io.IOException e) {
            throw new RuntimeException("读取 JS 文件时发生错误: " + filePath, e);
        } catch (EcmaError e) {
            throw new RuntimeException("JS 文件执行错误: " + e.getMessage() + " (行 " + e.lineNumber() + ")", e);
        } catch (Exception e) {
            throw new RuntimeException("执行 JS 文件时发生错误: " + filePath, e);
        }
    }
    
    /**
     * 序列化 Lua 值为 JSON 字符串
     * @param luaValue Lua 值
     * @return JSON 字符串
     */
    public String serialize(Object luaValue) {
        if (luaValue == null) {
            return "null";
        }
        try {
            // 使用现有的 stringify 方法来序列化
            return stringify(luaValue);
        } catch (Exception e) {
            throw new RuntimeException("序列化失败: " + e.getMessage(), e);
        }
    }
    
    /**
     * 将 Lua 值转换为 JavaScript 对象
     * @param luaValue Lua 值
     * @param scope JavaScript 作用域
     * @return JavaScript 对象
     */
    public Object toJSObject(Object luaValue, Scriptable scope) {
        if (luaValue == null) {
            return null;
        }
        try {
            // 处理不同类型的 Lua 值
            if (luaValue instanceof LuaObject) {
                LuaObject luaObj = (LuaObject) luaValue;
                LuaState L = luaObj.getLuaState();
                
                // 检查是否为表
                luaObj.push();
                if (L.type(-1) == LuaState.LUA_TTABLE) {
                    // 转换表为 JavaScript 对象
                    Scriptable jsObject = context.newObject(scope);
                    
                    // 遍历表的键值对
                    L.pushNil();
                    while (L.next(-2) != 0) {
                        String key;
                        int keyType = L.type(-2);
                        
                        if (keyType == LuaState.LUA_TSTRING) {
                            key = L.toString(-2);
                        } else if (keyType == LuaState.LUA_TNUMBER) {
                            double numKey = L.toNumber(-2);
                            if (numKey == (long) numKey) {
                                key = String.valueOf((long) numKey);
                            } else {
                                key = String.valueOf(numKey);
                            }
                        } else {
                            L.pop(1);
                            continue;
                        }
                        
                        // 直接获取原始值，而不是通过 convertLuaStackValueToJs
                        int valueType = L.type(-1);
                        Object value;
                        
                        if (valueType == LuaState.LUA_TSTRING) {
                            value = Context.javaToJS(L.toString(-1), scope);
                        } else if (valueType == LuaState.LUA_TNUMBER) {
                            value = Context.javaToJS(L.toNumber(-1), scope);
                        } else if (valueType == LuaState.LUA_TBOOLEAN) {
                            value = Context.javaToJS(L.toBoolean(-1), scope);
                        } else if (valueType == LuaState.LUA_TTABLE) {
                            // 递归处理嵌套表
                            LuaObject nestedObj = L.getLuaObject(-1);
                            value = toJSObject(nestedObj, scope);
                        } else if (valueType == LuaState.LUA_TNIL) {
                            value = null;
                        } else {
                            value = Context.javaToJS(L.toString(-1), scope);
                        }
                        
                        jsObject.put(key, jsObject, value);
                        
                        L.pop(1);
                    }
                    
                    L.pop(1);
                    return jsObject;
                } else {
                    L.pop(1);
                    return Context.javaToJS(luaValue, scope);
                }
            } else {
                return Context.javaToJS(luaValue, scope);
            }
        } catch (Exception e) {
            throw new RuntimeException("转换为 JS 对象失败: " + e.getMessage(), e);
        }
    }

    /**
     * 设置全局变量
     * @param name 变量名
     * @param value 变量值
     */
    public void set(String name, Object value) {
        if (name == null || name.isEmpty()) {
            throw new IllegalArgumentException("变量名不能为空");
        }
        Object jsValue = Context.javaToJS(value, scope);
        ScriptableObject.putProperty(scope, name, jsValue);
    }

    /**
     * 获取全局变量
     * @param name 变量名
     * @return 变量值
     */
    public Object get(String name) {
        if (name == null || name.isEmpty()) {
            throw new IllegalArgumentException("变量名不能为空");
        }
        Object jsValue = ScriptableObject.getProperty(scope, name);
        if (jsValue == Scriptable.NOT_FOUND) {
            return null;
        }
        return Context.jsToJava(jsValue, Object.class);
    }

    /**
     * 调用 JavaScript 函数
     * @param functionName 函数名
     * @param args 函数参数
     * @return 函数返回值
     */
    public Object call(String functionName, Object... args) {
        if (functionName == null || functionName.isEmpty()) {
            throw new IllegalArgumentException("函数名不能为空");
        }
        try {
            Object functionObj = ScriptableObject.getProperty(scope, functionName);
            if (!(functionObj instanceof Function)) {
                throw new RuntimeException("'" + functionName + "' 不是一个函数");
            }
            Function function = (Function) functionObj;

            Object[] jsArgs = new Object[args.length];
            for (int i = 0; i < args.length; i++) {
                jsArgs[i] = Context.javaToJS(args[i], scope);
            }

            Object result = function.call(context, scope, scope, jsArgs);
            return Context.jsToJava(result, Object.class);
        } catch (EcmaError e) {
            throw new RuntimeException("调用 JS 函数错误: " + e.getMessage() + " (行 " + e.lineNumber() + ")", e);
        } catch (Exception e) {
            throw new RuntimeException("调用 JS 函数时发生错误: " + functionName, e);
        }
    }

    /**
     * 创建新的 JavaScript 对象
     * @return JavaScript 对象
     */
    public Scriptable obj() {
        return context.newObject(scope);
    }

    /**
     * 创建新的 JavaScript 数组
     * @param length 数组长度
     * @return JavaScript 数组
     */
    public Object array(int length) {
        return context.newArray(scope, length);
    }

    /**
     * 创建 JavaScript 数组
     * @param elements 数组元素
     * @return JavaScript 数组
     */
    public Object array(Object... elements) {
        try {
            return new NativeArray(elements);
        } catch (Exception e) {
            throw new RuntimeException("创建数组失败: " + e.getMessage(), e);
        }
    }

    /**
     * 在 JavaScript 中导入 Java 类
     * @param className Java 类的全限定名
     * @return 导入的 Java 类在 JavaScript 中的表示
     */
    public Object importClass(String className) {
        if (className == null || className.isEmpty()) {
            throw new IllegalArgumentException("类名不能为空");
        }
        try {
            Class<?> javaClass = Class.forName(className);
            NativeJavaClass nativeJavaClass = new NativeJavaClass(scope, javaClass);
            String simpleName = javaClass.getSimpleName();
            ScriptableObject.putProperty(scope, simpleName, nativeJavaClass);
            return nativeJavaClass;
        } catch (ClassNotFoundException e) {
            throw new RuntimeException("找不到 Java 类: " + className, e);
        } catch (Exception e) {
            throw new RuntimeException("导入 Java 类时发生错误: " + className, e);
        }
    }

    /**
     * 将 Java 对象包装为 JavaScript 对象
     * @param javaObject 要包装的 Java 对象
     * @return 包装后的 JavaScript 对象
     */
    public Object wrap(Object javaObject) {
        if (javaObject == null) {
            return null;
        }
        try {
            return Context.toObject(javaObject, scope);
        } catch (Exception e) {
            throw new RuntimeException("包装 Java 对象时发生错误: " + e.getMessage(), e);
        }
    }

    /**
     * 将 JavaScript 对象转换为 Java 对象
     * @param jsObject 要转换的 JavaScript 对象
     * @param javaClass 目标 Java 类
     * @return 转换后的 Java 对象
     */
    public Object convert(Object jsObject, Class<?> javaClass) {
        if (jsObject == null) {
            return null;
        }
        try {
            return Context.jsToJava(jsObject, javaClass);
        } catch (Exception e) {
            throw new RuntimeException("转换为 Java 对象时发生错误: " + e.getMessage(), e);
        }
    }

    /**
     * 将 Lua 值转换为 JavaScript 值
     * @param luaValue Lua 值
     * @return 对应的 JavaScript 值
     */
    public Object toJs(Object luaValue) {
        if (luaValue == null) {
            return null;
        }

        if (luaValue instanceof LuaObject) {
            LuaObject luaObj = (LuaObject) luaValue;
            LuaState L = luaObj.getLuaState();

            int objIndex = L.getTop() + 1;
            luaObj.push();

            if (L.type(objIndex) != LuaState.LUA_TTABLE) {
                L.pop(1);
                return Context.javaToJS(luaValue, scope);
            }

            Scriptable jsObject = context.newObject(scope);

            L.pushNil();
            while (L.next(objIndex) != 0) {
                String key;
                int keyType = L.type(objIndex + 2);

                if (keyType == LuaState.LUA_TSTRING) {
                    key = L.toString(objIndex + 2);
                } else if (keyType == LuaState.LUA_TNUMBER) {
                    double numKey = L.toNumber(objIndex + 2);
                    if (numKey == (long) numKey) {
                        key = String.valueOf((long) numKey);
                    } else {
                        key = String.valueOf(numKey);
                    }
                } else {
                    L.pop(1);
                    continue;
                }

                Object jsValue = convertLuaStackValueToJs(L, objIndex + 1);
                jsObject.put(key, jsObject, jsValue);

                L.pop(1);
            }

            L.pop(1);
            return jsObject;
        }

        return Context.javaToJS(luaValue, scope);
    }

    /**
     * 辅助方法：将 Lua 栈上的值转换为 JavaScript 值
     */
    private Object convertLuaStackValueToJs(LuaState L, int index) {
        int valueType = L.type(index);
        if (valueType == LuaState.LUA_TTABLE) {
            // 检查是否为数组
            boolean isArray = true;
            int length = 0;
            
            // 先遍历一次，检查是否为数组
            L.pushNil();
            while (L.next(index) != 0) {
                int keyType = L.type(-2);
                if (keyType != LuaState.LUA_TNUMBER) {
                    isArray = false;
                } else {
                    double numKey = L.toNumber(-2);
                    if (numKey != (long) numKey || numKey < 1) {
                        isArray = false;
                    } else {
                        long intKey = (long) numKey;
                        if (intKey > length) {
                            length = (int) intKey;
                        }
                    }
                }
                L.pop(1);
            }
            
            if (isArray) {
                // 转换为 JavaScript 数组
                Object[] array = new Object[length];
                for (int i = 1; i <= length; i++) {
                    L.pushNumber(i);
                    L.getTable(index);
                    array[i - 1] = convertLuaStackValueToJs(L, -1);
                    L.pop(1);
                }
                return context.newArray(scope, array);
            } else {
                // 转换为 JavaScript 对象
                Scriptable jsObject = context.newObject(scope);
                L.pushNil();
                while (L.next(index) != 0) {
                    String key;
                    int keyType = L.type(-2);
                    
                    if (keyType == LuaState.LUA_TSTRING) {
                        key = L.toString(-2);
                    } else if (keyType == LuaState.LUA_TNUMBER) {
                        double numKey = L.toNumber(-2);
                        if (numKey == (long) numKey) {
                            key = String.valueOf((long) numKey);
                        } else {
                            key = String.valueOf(numKey);
                        }
                    } else {
                        L.pop(1);
                        continue;
                    }
                    
                    Object value = convertLuaStackValueToJs(L, -1);
                    jsObject.put(key, jsObject, value);
                    L.pop(1);
                }
                return jsObject;
            }
        } else if (valueType == LuaState.LUA_TSTRING) {
            return Context.javaToJS(L.toString(index), scope);
        } else if (valueType == LuaState.LUA_TBOOLEAN) {
            return Context.javaToJS(L.toBoolean(index), scope);
        } else if (valueType == LuaState.LUA_TNUMBER) {
            return Context.javaToJS(L.toNumber(index), scope);
        } else if (valueType == LuaState.LUA_TNIL) {
            return null;
        } else if (valueType == LuaState.LUA_TFUNCTION) {
            // 将 Lua 函数转换为 JavaScript 函数
            final LuaObject luaFunc = L.getLuaObject(index);
            return new org.mozilla.javascript.BaseFunction() {
                @Override
                public Object call(org.mozilla.javascript.Context cx, Scriptable scope,
                                  Scriptable thisObj, Object[] args) {
                    List<Object> javaArgs = new ArrayList<>();
                    for (Object arg : args) {
                        javaArgs.add(arg);
                    }
                    return JavaScriptEngine.this.callLua(luaFunc.toString(), javaArgs);
                }
            };
        }
        return Context.javaToJS(L.toString(index), scope);
    }

    /**
     * 将 JavaScript 值转换为 Lua 值
     * @param jsValue JavaScript 值
     * @return 对应的 Lua 值
     */
    public void toLua(Object jsValue) {
        LuaState L = luaContext.getLuaState();

        if (jsValue == null || jsValue == Undefined.instance) {
            L.pushNil();
            return;
        }

        if (jsValue instanceof Scriptable) {
            Scriptable scriptable = (Scriptable) jsValue;

            Object lengthProp = ScriptableObject.getProperty(scriptable, "length");
            boolean isArray = lengthProp instanceof Number && ((Number) lengthProp).intValue() >= 0;

            L.createTable(0, 0);

            Object[] ids = scriptable.getIds();
            for (Object id : ids) {
                String key;
                if (id instanceof String) {
                    key = (String) id;
                } else if (id instanceof Number) {
                    int numKey = ((Number) id).intValue();
                    if (isArray && numKey >= 0) {
                        key = String.valueOf(numKey);
                    } else {
                        key = String.valueOf(id);
                    }
                } else {
                    continue;
                }

                Object value = ScriptableObject.getProperty(scriptable, key);
                pushJsToLua(L, value);

                if (isArray && id instanceof Number) {
                    int numKey = ((Number) id).intValue();
                    L.pushNumber(numKey + 1);
                } else {
                    L.pushString(key);
                }
                L.setTable(-3);
            }
        } else if (jsValue instanceof String) {
            L.pushString((String) jsValue);
        } else if (jsValue instanceof Number) {
            L.pushNumber(((Number) jsValue).doubleValue());
        } else if (jsValue instanceof Boolean) {
            L.pushBoolean((Boolean) jsValue);
        } else {
            L.pushString(String.valueOf(jsValue));
        }
    }

    /**
     * 辅助方法：将 JavaScript 值推送到 Lua 栈
     */
    private void pushJsToLua(LuaState L, Object jsValue) {
        if (jsValue == null || jsValue == Undefined.instance) {
            L.pushNil();
        } else if (jsValue instanceof Scriptable) {
            Scriptable scriptable = (Scriptable) jsValue;
            
            // 检查是否为数组
            Object lengthProp = ScriptableObject.getProperty(scriptable, "length");
            boolean isArray = lengthProp instanceof Number && ((Number) lengthProp).intValue() >= 0;
            
            if (isArray) {
                // 处理数组
                int length = ((Number) lengthProp).intValue();
                L.createTable(length, 0);
                
                for (int i = 0; i < length; i++) {
                    Object element = ScriptableObject.getProperty(scriptable, Integer.toString(i));
                    pushJsToLua(L, element);
                    L.pushNumber(i + 1); // Lua数组从1开始
                    L.setTable(-3);
                }
            } else {
                // 处理普通对象
                L.createTable(0, 0);
                
                // 获取对象的所有属性
                Object[] ids = scriptable.getIds();
                for (Object id : ids) {
                    String key;
                    if (id instanceof String) {
                        key = (String) id;
                    } else if (id instanceof Number) {
                        key = String.valueOf(id);
                    } else {
                        continue;
                    }
                    
                    Object value = ScriptableObject.getProperty(scriptable, key);
                    pushJsToLua(L, value);
                    L.pushString(key);
                    L.setTable(-3);
                }
            }
        } else if (jsValue instanceof String) {
            L.pushString((String) jsValue);
        } else if (jsValue instanceof Number) {
            L.pushNumber(((Number) jsValue).doubleValue());
        } else if (jsValue instanceof Boolean) {
            L.pushBoolean((Boolean) jsValue);
        } else {
            L.pushString(String.valueOf(jsValue));
        }
    }

    /**
     * 设置 JavaScript 版本
     * @param version JavaScript 版本
     */
    public void setVersion(int version) {
        context.setLanguageVersion(version);
    }

    /**
     * 设置优化级别
     * @param level 优化级别，-1 表示关闭优化
     */
    public void setOptimizationLevel(int level) {
        context.setOptimizationLevel(level);
    }

    /**
     * 获取当前上下文
     * @return Rhino 上下文
     */
    public Context getContext() {
        return context;
    }

    /**
     * 获取当前作用域
     * @return JavaScript 作用域
     */
    public Scriptable getScope() {
        return scope;
    }

    /**
     * 获取引擎版本
     * @return 版本信息
     */
    public String version() {
        return "Rhino 1.9.0";
    }

    /**
     * 安全执行 JavaScript 代码
     * @param code 要执行的 JavaScript 代码
     * @return 包含执行结果和错误信息的 Map
     */
    public Map<String, Object> safeEval(String code) {
        HashMap<String, Object> result = new HashMap<>();
        try {
            Object evalResult = eval(code);
            result.put("success", true);
            result.put("result", evalResult);
        } catch (Exception e) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            PrintStream ps = new PrintStream(baos);
            e.printStackTrace(ps);
            ps.close();
            result.put("success", false);
            result.put("error", e.getMessage());
            result.put("stack", baos.toString());
        }
        return result;
    }

    /**
     * JSON 序列化
     * @param object 要序列化的对象
     * @return JSON 字符串
     */
    public String stringify(Object object) {
        if (object == null) {
            return "null";
        }
        try {
            if (object instanceof LuaObject) {
                LuaObject luaObj = (LuaObject) object;
                LuaState L = luaObj.getLuaState();
                
                int objIndex = L.getTop() + 1;
                luaObj.push();
                
                if (L.type(objIndex) != LuaState.LUA_TTABLE) {
                    L.pop(1);
                    return String.valueOf(object);
                }
                
                Object jsObject = context.newObject(scope);
                Scriptable scriptable = (Scriptable) jsObject;
                
                L.pushNil();
                while (L.next(objIndex) != 0) {
                    String key;
                    int keyType = L.type(objIndex + 2);
                    
                    if (keyType == LuaState.LUA_TSTRING) {
                        key = L.toString(objIndex + 2);
                    } else if (keyType == LuaState.LUA_TNUMBER) {
                        key = String.valueOf(L.toNumber(objIndex + 2));
                    } else {
                        L.pop(1);
                        continue;
                    }
                    
                    Object value;
                    int valueType = L.type(objIndex + 1);
                    if (valueType == LuaState.LUA_TTABLE) {
                        LuaObject nestedLuaObj = L.getLuaObject(objIndex + 1);
                        String nestedJson = stringify(nestedLuaObj);
                        value = eval("(" + nestedJson + ")");
                    } else if (valueType == LuaState.LUA_TSTRING) {
                        value = L.toString(objIndex + 1);
                    } else if (valueType == LuaState.LUA_TBOOLEAN) {
                        value = L.toBoolean(objIndex + 1);
                    } else if (valueType == LuaState.LUA_TNUMBER) {
                        value = L.toNumber(objIndex + 1);
                    } else if (valueType == LuaState.LUA_TNIL) {
                        value = null;
                    } else {
                        value = L.toString(objIndex + 1);
                    }
                    
                    Object jsValue = Context.javaToJS(value, scope);
                    scriptable.put(key, scriptable, jsValue);
                    
                    L.pop(1);
                }
                
                L.pop(1);
                
                Object jsonObj = ScriptableObject.getProperty(scope, "JSON");
                Function stringifyFunc = (Function) ScriptableObject.getProperty((Scriptable) jsonObj, "stringify");
                Object jsonResult = stringifyFunc.call(context, scope, (Scriptable) jsonObj, new Object[]{jsObject});
                return String.valueOf(jsonResult);
            }
            
            Scriptable jsObject = Context.toObject(object, scope);
            Object jsonObj = ScriptableObject.getProperty(scope, "JSON");
            Function stringifyFunc = (Function) ScriptableObject.getProperty((Scriptable) jsonObj, "stringify");
            Object jsonResult = stringifyFunc.call(context, scope, (Scriptable) jsonObj, new Object[]{jsObject});
            return String.valueOf(jsonResult);
        } catch (Exception e) {
            throw new RuntimeException("JSON 序列化失败: " + e.getMessage(), e);
        }
    }

    /**
     * JSON 反序列化
     * @param json JSON 字符串
     * @return 反序列化后的对象
     */
    public Object parse(String json) {
        if (json == null || json.isEmpty()) {
            return null;
        }
        try {
            Object jsonObj = ScriptableObject.getProperty(scope, "JSON");
            Function parseFunc = (Function) ScriptableObject.getProperty((Scriptable) jsonObj, "parse");
            return parseFunc.call(context, scope, (Scriptable) jsonObj, new Object[]{json});
        } catch (Exception e) {
            throw new RuntimeException("JSON 反序列化失败: " + e.getMessage(), e);
        }
    }

    /**
     * 创建正则表达式对象
     * @param pattern 正则表达式模式
     * @return 正则表达式对象
     */
    public Object regexp(String pattern) {
        return regexp(pattern, "");
    }

    /**
     * 创建正则表达式对象
     * @param pattern 正则表达式模式
     * @param flags 正则表达式标志
     * @return 正则表达式对象
     */
    public Object regexp(String pattern, String flags) {
        if (pattern == null) {
            throw new IllegalArgumentException("正则表达式模式不能为 null");
        }
        try {
            Function regExpCtor = (Function) ScriptableObject.getProperty(scope, "RegExp");
            return regExpCtor.construct(context, scope, new Object[]{pattern, flags});
        } catch (Exception e) {
            throw new RuntimeException("创建正则表达式失败: " + e.getMessage(), e);
        }
    }

    /**
     * 测试正则表达式
     * @param regex 正则表达式对象
     * @param str 要测试的字符串
     * @return 测试结果
     */
    public boolean test(Object regex, String str) {
        if (regex == null || str == null) {
            throw new IllegalArgumentException("参数不能为 null");
        }
        try {
            Function testFunction = (Function) ScriptableObject.getProperty((Scriptable) regex, "test");
            Object result = testFunction.call(context, scope, (Scriptable) regex, new Object[]{str});
            return Boolean.parseBoolean(String.valueOf(result));
        } catch (Exception e) {
            throw new RuntimeException("正则表达式测试失败: " + e.getMessage(), e);
        }
    }

    /**
     * 使用正则表达式匹配
     * @param regex 正则表达式对象
     * @param str 要匹配的字符串
     * @return 匹配结果
     */
    public Object match(Object regex, String str) {
        if (regex == null || str == null) {
            throw new IllegalArgumentException("参数不能为 null");
        }
        try {
            Function matchFunction = (Function) ScriptableObject.getProperty((Scriptable) regex, "match");
            return matchFunction.call(context, scope, (Scriptable) regex, new Object[]{str});
        } catch (Exception e) {
            throw new RuntimeException("正则表达式匹配失败: " + e.getMessage(), e);
        }
    }

    /**
     * 将 JavaScript 数组转换为 Java 列表
     * @param jsArray JavaScript 数组
     * @return Java 列表
     */
    public List<Object> toList(Object jsArray) {
        if (jsArray == null) {
            return new ArrayList<>();
        }
        try {
            NativeArray nativeArray = (NativeArray) jsArray;
            List<Object> javaList = new ArrayList<>();
            for (Object key : nativeArray.getIds()) {
                if (key instanceof Number) {
                    int index = ((Number) key).intValue();
                    javaList.add(nativeArray.get(index, nativeArray));
                }
            }
            return javaList;
        } catch (Exception e) {
            throw new RuntimeException("转换为 Java 列表失败: " + e.getMessage(), e);
        }
    }

    /**
     * 设置对象属性
     * @param obj JavaScript 对象
     * @param key 属性名
     * @param value 属性值
     */
    public void setProp(Object obj, String key, Object value) {
        if (obj == null || key == null) {
            throw new IllegalArgumentException("对象或键名不能为 null");
        }
        try {
            ScriptableObject.putProperty((Scriptable) obj, key, Context.javaToJS(value, scope));
        } catch (Exception e) {
            throw new RuntimeException("设置对象属性失败: " + e.getMessage(), e);
        }
    }

    /**
     * 获取对象属性
     * @param obj JavaScript 对象
     * @param key 属性名
     * @return 属性值
     */
    public Object getProp(Object obj, String key) {
        if (obj == null || key == null) {
            throw new IllegalArgumentException("对象或键名不能为 null");
        }
        try {
            Object jsValue = ScriptableObject.getProperty((Scriptable) obj, key);
            return Context.jsToJava(jsValue, Object.class);
        } catch (Exception e) {
            throw new RuntimeException("获取对象属性失败: " + e.getMessage(), e);
        }
    }

    /**
     * 删除对象属性
     * @param obj JavaScript 对象
     * @param key 属性名
     * @return 是否删除成功
     */
    public boolean delete(Object obj, String key) {
        if (obj == null || key == null) {
            throw new IllegalArgumentException("对象或键名不能为 null");
        }
        try {
            ((Scriptable) obj).delete(key);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    /**
     * 获取对象的所有键
     * @param obj JavaScript 对象
     * @return 键的数组
     */
    public Object keys(Object obj) {
        if (obj == null) {
            return null;
        }
        try {
            Scriptable scriptable = (Scriptable) obj;
            Object[] ids = scriptable.getIds();
            return new NativeArray(ids);
        } catch (Exception e) {
            throw new RuntimeException("获取对象键失败: " + e.getMessage(), e);
        }
    }

    /**
     * 检查对象是否为数组
     * @param obj 要检查的对象
     * @return 是否为数组
     */
    public boolean isArray(Object obj) {
        if (obj == null) {
            return false;
        }
        return obj instanceof NativeArray;
    }

    /**
     * 检查对象是否为普通对象
     * @param obj 要检查的对象
     * @return 是否为普通对象（非数组、非函数）
     */
    public boolean isObject(Object obj) {
        if (obj == null) {
            return false;
        }
        return obj instanceof Scriptable && !(obj instanceof NativeArray) && !(obj instanceof Function);
    }

    /**
     * 检查对象是否为函数
     * @param obj 要检查的对象
     * @return 是否为函数
     */
    public boolean isFunction(Object obj) {
        return obj instanceof Function;
    }

    /**
     * 检查对象是否为字符串
     * @param obj 要检查的对象
     * @return 是否为字符串
     */
    public boolean isString(Object obj) {
        return obj instanceof String;
    }

    /**
     * 检查对象是否为数字
     * @param obj 要检查的对象
     * @return 是否为数字
     */
    public boolean isNumber(Object obj) {
        return obj instanceof Number;
    }

    /**
     * 检查对象是否为布尔值
     * @param obj 要检查的对象
     * @return 是否为布尔值
     */
    public boolean isBoolean(Object obj) {
        return obj instanceof Boolean;
    }

    /**
     * 检查对象是否为 null 或 undefined
     * @param obj 要检查的对象
     * @return 是否为 null 或 undefined
     */
    public boolean isNull(Object obj) {
        return obj == null || obj == Undefined.instance;
    }

    /**
     * 将 JavaScript 值转换为指定类型
     * @param jsValue JavaScript 值
     * @param targetType 目标类型
     * @return 转换后的值
     */
    public Object convertJsToType(Object jsValue, Class<?> targetType) {
        if (jsValue == null || jsValue == Undefined.instance) {
            return null;
        }
        
        try {
            if (targetType == String.class) {
                return String.valueOf(jsValue);
            } else if (targetType == Integer.class || targetType == int.class) {
                if (jsValue instanceof Number) {
                    return ((Number) jsValue).intValue();
                } else if (jsValue instanceof String) {
                    return Integer.parseInt((String) jsValue);
                }
            } else if (targetType == Double.class || targetType == double.class) {
                if (jsValue instanceof Number) {
                    return ((Number) jsValue).doubleValue();
                } else if (jsValue instanceof String) {
                    return Double.parseDouble((String) jsValue);
                }
            } else if (targetType == Boolean.class || targetType == boolean.class) {
                if (jsValue instanceof Boolean) {
                    return jsValue;
                } else if (jsValue instanceof String) {
                    return Boolean.parseBoolean((String) jsValue);
                } else if (jsValue instanceof Number) {
                    return ((Number) jsValue).doubleValue() != 0;
                }
            } else if (targetType == List.class) {
                if (jsValue instanceof NativeArray) {
                    NativeArray array = (NativeArray) jsValue;
                    List<Object> list = new ArrayList<>();
                    for (Object key : array.getIds()) {
                        if (key instanceof Number) {
                            int index = ((Number) key).intValue();
                            list.add(array.get(index, array));
                        }
                    }
                    return list;
                }
            } else if (targetType == Map.class) {
                if (jsValue instanceof Scriptable) {
                    Scriptable scriptable = (Scriptable) jsValue;
                    Map<String, Object> map = new HashMap<>();
                    for (Object id : scriptable.getIds()) {
                        if (id instanceof String) {
                            String key = (String) id;
                            map.put(key, scriptable.get(key, scriptable));
                        }
                    }
                    return map;
                }
            }
            
            return Context.jsToJava(jsValue, targetType);
        } catch (Exception e) {
            throw new RuntimeException("类型转换失败: " + e.getMessage(), e);
        }
    }

    /**
     * 将 Lua 值转换为指定类型
     * @param luaValue Lua 值
     * @param targetType 目标类型
     * @return 转换后的值
     */
    public Object convertLuaToType(Object luaValue, Class<?> targetType) {
        if (luaValue == null) {
            return null;
        }
        
        try {
            if (targetType == String.class) {
                return String.valueOf(luaValue);
            } else if (targetType == Integer.class || targetType == int.class) {
                if (luaValue instanceof Number) {
                    return ((Number) luaValue).intValue();
                } else if (luaValue instanceof String) {
                    return Integer.parseInt((String) luaValue);
                }
            } else if (targetType == Double.class || targetType == double.class) {
                if (luaValue instanceof Number) {
                    return ((Number) luaValue).doubleValue();
                } else if (luaValue instanceof String) {
                    return Double.parseDouble((String) luaValue);
                }
            } else if (targetType == Boolean.class || targetType == boolean.class) {
                if (luaValue instanceof Boolean) {
                    return luaValue;
                } else if (luaValue instanceof String) {
                    return Boolean.parseBoolean((String) luaValue);
                } else if (luaValue instanceof Number) {
                    return ((Number) luaValue).doubleValue() != 0;
                }
            } else if (targetType == List.class) {
                if (luaValue instanceof LuaObject) {
                    LuaObject luaObj = (LuaObject) luaValue;
                    LuaState L = luaObj.getLuaState();
                    
                    luaObj.push();
                    if (L.type(-1) == LuaState.LUA_TTABLE) {
                        List<Object> list = new ArrayList<>();
                        L.pushNil();
                        while (L.next(-2) != 0) {
                            list.add(popLuaValue(L, -1));
                            L.pop(1);
                        }
                        L.pop(1);
                        return list;
                    }
                    L.pop(1);
                }
            } else if (targetType == Map.class) {
                if (luaValue instanceof LuaObject) {
                    LuaObject luaObj = (LuaObject) luaValue;
                    LuaState L = luaObj.getLuaState();
                    
                    luaObj.push();
                    if (L.type(-1) == LuaState.LUA_TTABLE) {
                        Map<String, Object> map = new HashMap<>();
                        L.pushNil();
                        while (L.next(-2) != 0) {
                            String key = L.toString(-2);
                            Object value = popLuaValue(L, -1);
                            map.put(key, value);
                            L.pop(1);
                        }
                        L.pop(1);
                        return map;
                    }
                    L.pop(1);
                }
            }
            
            return luaValue;
        } catch (Exception e) {
            throw new RuntimeException("类型转换失败: " + e.getMessage(), e);
        }
    }

    /**
     * 获取当前时间戳（毫秒）
     * @return 当前时间戳
     */
    public long time() {
        return System.currentTimeMillis();
    }

    /**
     * 加载 JavaScript 模块
     * @param modulePath 模块路径
     * @return 加载的模块对象
     */
    public Object loadModule(String modulePath) {
        try {
            return evalFile(modulePath);
        } catch (Exception e) {
            throw new RuntimeException("加载模块失败: " + e.getMessage(), e);
        }
    }

    /**
     * 定义 JavaScript 模块
     * @param moduleName 模块名
     * @param moduleObject 模块对象
     */
    public void defineModule(String moduleName, Object moduleObject) {
        try {
            ScriptableObject.putProperty(scope, moduleName, moduleObject);
        } catch (Exception e) {
            throw new RuntimeException("定义模块失败: " + e.getMessage(), e);
        }
    }

    /**
     * 获取 JavaScript 环境信息
     * @return 环境信息对象
     */
    public Map<String, Object> info() {
        HashMap<String, Object> info = new HashMap<>();
        info.put("version", version());
        info.put("languageVersion", context.getLanguageVersion());
        info.put("optimizationLevel", context.getOptimizationLevel());
        info.put("sandbox", sandboxMode);
        return info;
    }

    /**
     * 释放资源
     */
    public void close() {
        if (context != null) {
            Context.exit();
            context = null;
        }
    }

    /**
     * 重置引擎状态
     */
    public void reset() {
        close();
        init();
    }

    /**
     * 将 Lua 函数注册到 JavaScript
     * @param luaFunctionName Lua 函数名
     * @param luaFunction Lua 函数对象
     */
    public void bindLua(String luaFunctionName, Object luaFunction) {
        if (luaFunctionName == null || luaFunctionName.isEmpty()) {
            throw new IllegalArgumentException("函数名不能为空");
        }
        try {
            LuaState L = luaContext.getLuaState();
            
            if (luaFunction instanceof LuaObject) {
                LuaObject luaObj = (LuaObject) luaFunction;
                luaObj.push();
            } else {
                throw new RuntimeException("不是有效的 Lua 函数对象");
            }
            
            if (L.type(-1) != LuaState.LUA_TFUNCTION) {
                L.pop(1);
                throw new RuntimeException("指定的不是 Lua 函数");
            }
            
            L.setGlobal(luaFunctionName);
            
            String wrapperCode = "function " + luaFunctionName + "() { " +
                "var args = Array.prototype.slice.call(arguments);" +
                "var result = lua.call('" + luaFunctionName + "', java.util.Arrays.asList(args));" +
                "return result;" +
                "}";
            
            context.evaluateString(scope, wrapperCode, "lua_wrapper_" + luaFunctionName, 1, null);
            
        } catch (Exception e) {
            throw new RuntimeException("注册 Lua 函数到 JS 失败: " + e.getMessage(), e);
        }
    }

    /**
     * 将 JavaScript 函数注册到 Lua
     * @param jsFunction JavaScript 函数对象
     * @return 可供 Lua 调用的包装器对象
     */
    public Object bind(Object jsFunction) {
        if (jsFunction == null) {
            throw new IllegalArgumentException("JavaScript 函数不能为 null");
        }
        try {
            LuaState L = luaContext.getLuaState();
            
            JavaFunction luaWrapper = new JavaFunction(L) {
                private final Object jsFunc;
                private final JavaScriptEngine engine;
                
                {
                    this.jsFunc = jsFunction;
                    this.engine = JavaScriptEngine.this;
                }
                
                @Override
                public int execute() throws LuaException {
                    try {
                        int top = L.getTop();
                        Object[] jsArgs = new Object[top - 1];
                        
                        for (int i = 2; i <= top; i++) {
                            int type = L.type(i);
                            if (type == LuaState.LUA_TSTRING) {
                                jsArgs[i - 2] = L.toString(i);
                            } else if (type == LuaState.LUA_TNUMBER) {
                                jsArgs[i - 2] = L.toNumber(i);
                            } else if (type == LuaState.LUA_TBOOLEAN) {
                                jsArgs[i - 2] = L.toBoolean(i);
                            } else if (type == LuaState.LUA_TTABLE) {
                                jsArgs[i - 2] = engine.stringify(L.getLuaObject(i));
                            } else if (type == LuaState.LUA_TNIL) {
                                jsArgs[i - 2] = null;
                            } else {
                                jsArgs[i - 2] = L.toString(i);
                            }
                        }
                        
                        if (jsFunc instanceof Function) {
                            Function func = (Function) jsFunc;
                            Object result = func.call(context, scope, scope, jsArgs);
                            
                            if (result == null || result == Undefined.instance) {
                                L.pushNil();
                            } else if (result instanceof Number) {
                                L.pushNumber(((Number) result).doubleValue());
                            } else if (result instanceof Boolean) {
                                L.pushBoolean((Boolean) result);
                            } else if (result instanceof String) {
                                L.pushString((String) result);
                            } else if (result instanceof Scriptable) {
                                engine.toLua(result);
                            } else {
                                L.pushString(String.valueOf(result));
                            }
                        } else {
                            L.pushString(String.valueOf(jsFunc));
                        }
                        
                        return 1;
                    } catch (Exception e) {
                        throw new LuaException("调用 JS 函数失败: " + e.getMessage());
                    }
                }
            };
            
            return luaWrapper;
            
        } catch (Exception e) {
            throw new RuntimeException("注册 JS 函数到 Lua 失败: " + e.getMessage(), e);
        }
    }
    /**
     * 直接在 JavaScript 中调用 Lua 函数
     * @param funcName 函数名
     * @param args 参数列表
     * @return 调用结果
     */
    public Object callLua(String funcName, List<Object> args) {
        if (funcName == null || funcName.isEmpty()) {
            throw new IllegalArgumentException("函数名不能为空");
        }
        if (args == null) {
            args = new ArrayList<>();
        }
        try {
            LuaState L = luaContext.getLuaState();
            
            // 尝试从全局表中查找函数
            L.getGlobal(funcName);
            if (L.type(-1) == LuaState.LUA_TFUNCTION) {
                // 找到函数，执行调用
                for (Object arg : args) {
                    pushJavaArgToLua(L, arg);
                }
                
                int result = L.pcall(args.size(), LuaState.LUA_MULTRET, 0);
                if (result != 0) {
                    String error = L.toString(-1);
                    L.pop(1);
                    throw new RuntimeException("调用 Lua 函数失败: " + error);
                }
                
                int returnCount = L.getTop();
                if (returnCount == 0) {
                    return null;
                } else if (returnCount == 1) {
                    Object value = popLuaValue(L, -1);
                    L.pop(1);
                    return value;
                } else {
                    Object[] results = new Object[returnCount];
                    for (int i = 0; i < returnCount; i++) {
                        results[i] = popLuaValue(L, i + 1);
                    }
                    L.pop(returnCount);
                    return results;
                }
            } else {
                L.pop(1);
                
                // 特殊处理 print 函数
                if ("print".equals(funcName)) {
                    // 直接使用 LuaContext 的 sendMsg 方法来实现 print 功能
                    StringBuilder sb = new StringBuilder();
                    for (int i = 0; i < args.size(); i++) {
                        if (i > 0) {
                            sb.append(" ");
                        }
                        sb.append(args.get(i));
                    }
                    luaContext.sendMsg(sb.toString());
                    return null;
                }
                
                // 其他函数不存在
                throw new RuntimeException("Lua 函数不存在: " + funcName);
            }
            
        } catch (RuntimeException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException("调用 Lua 函数失败: " + e.getMessage(), e);
        }
    }

    /**
     * 将 Java 参数推送到 Lua 栈
     */
    private void pushJavaArgToLua(LuaState L, Object arg) {
        if (arg == null) {
            L.pushNil();
        } else if (arg instanceof String) {
            L.pushString((String) arg);
        } else if (arg instanceof Number) {
            L.pushNumber(((Number) arg).doubleValue());
        } else if (arg instanceof Boolean) {
            L.pushBoolean((Boolean) arg);
        } else if (arg instanceof LuaObject) {
            ((LuaObject) arg).push();
        } else if (arg instanceof List) {
            List<?> list = (List<?>) arg;
            L.createTable(list.size(), 0);
            int i = 1;
            for (Object item : list) {
                pushJavaArgToLua(L, item);
                L.pushNumber(i++);
                L.setTable(-3);
            }
        } else if (arg instanceof Map) {
            Map<?, ?> map = (Map<?, ?>) arg;
            L.createTable(0, map.size());
            for (Object key : map.keySet()) {
                pushJavaArgToLua(L, map.get(key));
                L.pushString(String.valueOf(key));
                L.setTable(-3);
            }
        } else {
            L.pushString(String.valueOf(arg));
        }
    }

    /**
     * 从 Lua 栈弹出值
     */
    private Object popLuaValue(LuaState L, int index) {
        int type = L.type(index);
        if (type == LuaState.LUA_TSTRING) {
            return L.toString(index);
        } else if (type == LuaState.LUA_TNUMBER) {
            return L.toNumber(index);
        } else if (type == LuaState.LUA_TBOOLEAN) {
            return L.toBoolean(index);
        } else if (type == LuaState.LUA_TTABLE) {
            return L.getLuaObject(index);
        } else if (type == LuaState.LUA_TNIL) {
            return null;
        }
        return L.toString(index);
    }

    /**
     * 检查 JavaScript 值是否为 Promise
     * @param obj 要检查的对象
     * @return 是否为 Promise
     */
    public boolean isPromise(Object obj) {
        if (obj == null || !(obj instanceof Scriptable)) {
            return false;
        }
        Scriptable scriptable = (Scriptable) obj;
        Object then = ScriptableObject.getProperty(scriptable, "then");
        return then instanceof Function;
    }

    /**
     * 等待 Promise 完成（同步阻塞）
     * @param promise Promise 对象
     * @return Promise 的结果
     * @throws RuntimeException 如果 Promise 被拒绝
     */
    public Object await(Object promise) {
        if (!isPromise(promise)) {
            throw new IllegalArgumentException("对象不是 Promise");
        }
        try {
            Scriptable promiseScriptable = (Scriptable) promise;
            Object then = ScriptableObject.getProperty(promiseScriptable, "then");
            if (!(then instanceof Function)) {
                throw new RuntimeException("Promise 没有 then 方法");
            }

            final Object lock = new Object();
            final boolean[] completed = {false};

            JavaFunction callback = new JavaFunction(luaContext.getLuaState()) {
                @Override
                public int execute() throws LuaException {
                    synchronized (lock) {
                        int top = L.getTop();
                        if (top >= 2) {
                            int type = L.type(2);
                            if (type == LuaState.LUA_TFUNCTION) {
                                L.pushValue(2);
                                L.call(1, 0);
                                L.pop(1);
                            }
                        }
                        completed[0] = true;
                        lock.notifyAll();
                    }
                    return 0;
                }
            };

            Object[] thenArgs = new Object[]{callback, new org.mozilla.javascript.BaseFunction() {
                @Override
                public Object call(org.mozilla.javascript.Context cx, Scriptable scope,
                                  Scriptable thisObj, Object[] args) {
                    if (args.length > 0 && args[0] instanceof org.mozilla.javascript.EcmaError) {
                        throw new RuntimeException("Promise 被拒绝: " + args[0]);
                    }
                    return null;
                }
            }};

            Function thenFunc = (Function) then;
            thenFunc.call(context, scope, (Scriptable) promise, thenArgs);

            synchronized (lock) {
                while (!completed[0]) {
                    lock.wait();
                }
            }

            return null;
        } catch (InterruptedException e) {
            throw new RuntimeException("等待 Promise 被中断", e);
        } catch (Exception e) {
            throw new RuntimeException("等待 Promise 失败: " + e.getMessage(), e);
        }
    }

    /**
     * 获取全局对象
     * @return 全局对象
     */
    public Scriptable global() {
        return scope;
    }

    /**
     * 预编译 JavaScript 代码
     * @param code JavaScript 代码
     * @return 编译后的函数对象
     */
    public Function compile(String code) {
        if (code == null || code.isEmpty()) {
            throw new IllegalArgumentException("代码不能为空");
        }
        try {
            return context.compileFunction(scope, "compiled", code, 1, null);
        } catch (Exception e) {
            throw new RuntimeException("编译 JS 代码失败: " + e.getMessage(), e);
        }
    }

    /**
     * 预编译 JavaScript 代码
     * @param code JavaScript 代码
     * @param name 代码名称（用于调试）
     * @return 编译后的函数对象
     */
    public Function compile(String code, String name) {
        if (code == null || code.isEmpty()) {
            throw new IllegalArgumentException("代码不能为空");
        }
        try {
            return context.compileFunction(scope, name != null ? name : "compiled", code, 1, null);
        } catch (Exception e) {
            throw new RuntimeException("编译 JS 代码失败: " + e.getMessage(), e);
        }
    }
}
