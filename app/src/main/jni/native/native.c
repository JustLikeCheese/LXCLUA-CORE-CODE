#include "native.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>
#include "../lua/lopcodes.h"  // 包含指令操作相关的头文件

/**
* 获取当前时间戳
* @return 当前时间戳（秒）
*/
int native_get_current_time(lua_State *L) {
    time_t current_time = time(NULL);
    lua_pushinteger(L, (lua_Integer)current_time);
    return 1;
}

/**
* 简单的加法函数
* @param a: 第一个加数
* @param b: 第二个加数
* @return a + b
*/
int native_add(lua_State *L) {
    lua_Number a = luaL_checknumber(L, 1);
    lua_Number b = luaL_checknumber(L, 2);
    lua_pushnumber(L, a + b);
    return 1;
}

/**
* 打印日志到Android日志系统
* @param tag: 日志标签
* @param message: 日志消息
* @return 成功返回1，失败返回0
*/
int native_log(lua_State *L) {
    const char *tag = luaL_checkstring(L, 1);
    const char *message = luaL_checkstring(L, 2);
    __android_log_print(ANDROID_LOG_INFO, tag, "%s", message);
    lua_pushinteger(L, 1);
    return 1;
}

/**
* 获取模块版本信息
* @return 版本字符串
*/
int native_get_version(lua_State *L) {
    lua_pushstring(L, "1.0.0");
    return 1;
}

/**
* 字符串反转函数
* @param str: 要反转的字符串
* @return 反转后的字符串
*/
int native_reverse_string(lua_State *L) {
    size_t len;
    const char *str = luaL_checklstring(L, 1, &len);
    char *reversed = (char *)malloc(len + 1);
    
    if (reversed == NULL) {
        return luaL_error(L, "内存分配失败");
    }
    
    for (size_t i = 0; i < len; i++) {
        reversed[i] = str[len - 1 - i];
    }
    reversed[len] = '\0';
    
    lua_pushstring(L, reversed);
    free(reversed);
    return 1;
}

/**
* Lua底层API封装：获取栈顶索引
* @return 栈顶索引
*/
int native_gettop(lua_State *L) {
    int top = lua_gettop(L);
    lua_pushinteger(L, top);
    return 1;
}

/**
* Lua底层API封装：设置栈顶索引
* @param idx: 新的栈顶索引
*/
int native_settop(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_settop(L, idx);
    return 0;
}

/**
* Lua底层API封装：推送nil到栈
*/
int native_pushnil(lua_State *L) {
    lua_pushnil(L);
    return 1;
}

/**
* Lua底层API封装：推送布尔值到栈
* @param b: 布尔值
*/
int native_pushboolean(lua_State *L) {
    int b = lua_toboolean(L, 1);
    lua_pushboolean(L, b);
    return 1;
}

/**
* Lua底层API封装：推送整数到栈
* @param n: 整数值
*/
int native_pushinteger(lua_State *L) {
    lua_Integer n = luaL_checkinteger(L, 1);
    lua_pushinteger(L, n);
    return 1;
}

/**
* Lua底层API封装：推送数字到栈
* @param n: 数字值
*/
int native_pushnumber(lua_State *L) {
    lua_Number n = luaL_checknumber(L, 1);
    lua_pushnumber(L, n);
    return 1;
}

/**
* Lua底层API封装：推送字符串到栈
* @param s: 字符串
*/
int native_pushstring(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    lua_pushstring(L, s);
    return 1;
}

/**
* Lua底层API封装：推送指定长度的字符串到栈
* @param s: 字符串
* @param len: 长度
*/
int native_pushlstring(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    size_t len = luaL_checkinteger(L, 2);
    lua_pushlstring(L, s, len);
    return 1;
}

/**
* Lua底层API封装：推送C闭包到栈
* @param fn: C函数
* @param n: 上值数量
*/
int native_pushcclosure(lua_State *L) {
    lua_CFunction fn = (lua_CFunction)lua_touserdata(L, 1);
    int n = luaL_checkinteger(L, 2);
    lua_pushcclosure(L, fn, n);
    return 1;
}

/**
* Lua底层API封装：推送轻量级用户数据到栈
* @param p: 指针
*/
int native_pushlightuserdata(lua_State *L) {
    void *p = lua_touserdata(L, 1);
    lua_pushlightuserdata(L, p);
    return 1;
}

/**
* Lua底层API封装：推送当前线程到栈
* @return 1 if this thread is the main thread, 0 otherwise
*/
int native_pushthread(lua_State *L) {
    int result = lua_pushthread(L);
    lua_pushboolean(L, result);
    return 1;
}

/**
* Lua底层API封装：获取值的类型
* @param idx: 栈索引
* @return 类型码
*/
int native_type(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    int type = lua_type(L, idx);
    lua_pushinteger(L, type);
    return 1;
}

/**
* Lua底层API封装：获取类型名称
* @param tp: 类型码
* @return 类型名称
*/
int native_typename(lua_State *L) {
    int tp = luaL_checkinteger(L, 1);
    const char *name = lua_typename(L, tp);
    lua_pushstring(L, name);
    return 1;
}

/**
* Lua底层API封装：将值转换为数字
* @param idx: 栈索引
* @return 数字值
*/
int native_tonumber(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_Number num = lua_tonumber(L, idx);
    lua_pushnumber(L, num);
    return 1;
}

/**
* Lua底层API封装：将值转换为整数
* @param idx: 栈索引
* @return 整数值
*/
int native_tointeger(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_Integer num = lua_tointeger(L, idx);
    lua_pushinteger(L, num);
    return 1;
}

/**
* Lua底层API封装：将值转换为布尔值
* @param idx: 栈索引
* @return 布尔值
*/
int native_toboolean(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    int b = lua_toboolean(L, idx);
    lua_pushboolean(L, b);
    return 1;
}

/**
* Lua底层API封装：将值转换为字符串
* @param idx: 栈索引
* @return 字符串
*/
int native_tostring(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    const char *str = lua_tostring(L, idx);
    lua_pushstring(L, str);
    return 1;
}

/**
* Lua底层API封装：获取值的原始长度
* @param idx: 栈索引
* @return 长度
*/
int native_rawlen(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    size_t len = lua_rawlen(L, idx);
    lua_pushinteger(L, len);
    return 1;
}

/**
* Lua底层API封装：将值转换为用户数据
* @param idx: 栈索引
* @return 用户数据指针
*/
int native_touserdata(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    void *ud = lua_touserdata(L, idx);
    lua_pushlightuserdata(L, ud);
    return 1;
}

/**
* Lua底层API封装：将值转换为线程
* @param idx: 栈索引
* @return 线程指针
*/
int native_tothread(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_State *thread = lua_tothread(L, idx);
    lua_pushlightuserdata(L, thread);
    return 1;
}

/**
* Lua底层API封装：检查两个值是否原始相等
* @param idx1: 第一个值的索引
* @param idx2: 第二个值的索引
* @return 1 if equal, 0 otherwise
*/
int native_rawequal(lua_State *L) {
    int idx1 = luaL_checkinteger(L, 1);
    int idx2 = luaL_checkinteger(L, 2);
    int result = lua_rawequal(L, idx1, idx2);
    lua_pushboolean(L, result);
    return 1;
}

/**
* Lua底层API封装：比较两个值
* @param idx1: 第一个值的索引
* @param idx2: 第二个值的索引
* @param op: 比较操作符
* @return 1 if comparison is true, 0 otherwise
*/
int native_compare(lua_State *L) {
    int idx1 = luaL_checkinteger(L, 1);
    int idx2 = luaL_checkinteger(L, 2);
    int op = luaL_checkinteger(L, 3);
    int result = lua_compare(L, idx1, idx2, op);
    lua_pushboolean(L, result);
    return 1;
}

/**
* Lua底层API封装：执行算术操作
* @param op: 算术操作符
*/
int native_arith(lua_State *L) {
    int op = luaL_checkinteger(L, 1);
    lua_arith(L, op);
    return 0;
}

/**
* Lua底层API封装：表操作 - 获取表字段（使用元方法）
* @param idx: 表的索引
* @return 字段值
*/
int native_gettable(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_gettable(L, idx);
    return 1;
}

/**
* Lua底层API封装：表操作 - 获取字段
* @param table: 表（从栈中获取）
* @param key: 键
* @return 字段值
*/
int native_getfield(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    const char *key = luaL_checkstring(L, 2);
    lua_getfield(L, idx, key);
    return 1;
}

/**
* Lua底层API封装：表操作 - 获取整数索引字段
* @param table: 表（从栈中获取）
* @param n: 整数索引
* @return 字段值
*/
int native_geti(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_Integer n = luaL_checkinteger(L, 2);
    lua_geti(L, idx, n);
    return 1;
}

/**
* Lua底层API封装：表操作 - 原始获取（不使用元方法）
* @param table: 表（从栈中获取）
* @return 字段值
*/
int native_rawget(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_rawget(L, idx);
    return 1;
}

/**
* Lua底层API封装：表操作 - 原始获取整数索引字段
* @param table: 表（从栈中获取）
* @param n: 整数索引
* @return 字段值
*/
int native_rawgeti(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_Integer n = luaL_checkinteger(L, 2);
    lua_rawgeti(L, idx, n);
    return 1;
}

/**
* Lua底层API封装：表操作 - 设置表字段（使用元方法）
* @param idx: 表的索引
*/
int native_settable(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_settable(L, idx);
    return 0;
}

/**
* Lua底层API封装：表操作 - 设置字段
* @param table: 表（从栈中获取）
* @param key: 键
* @param value: 值（从栈顶获取）
*/
int native_setfield(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    const char *key = luaL_checkstring(L, 2);
    lua_setfield(L, idx, key);
    return 0;
}

/**
* Lua底层API封装：表操作 - 设置整数索引字段
* @param table: 表（从栈中获取）
* @param n: 整数索引
* @param value: 值（从栈顶获取）
*/
int native_seti(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_Integer n = luaL_checkinteger(L, 2);
    lua_seti(L, idx, n);
    return 0;
}

/**
* Lua底层API封装：表操作 - 原始设置（不使用元方法）
* @param table: 表（从栈中获取）
* @param value: 值（从栈顶获取）
*/
int native_rawset(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_rawset(L, idx);
    return 0;
}

/**
* Lua底层API封装：表操作 - 原始设置整数索引字段
* @param table: 表（从栈中获取）
* @param n: 整数索引
* @param value: 值（从栈顶获取）
*/
int native_rawseti(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_Integer n = luaL_checkinteger(L, 2);
    lua_rawseti(L, idx, n);
    return 0;
}

/**
* Lua底层API封装：创建表
* @param narr: 数组部分的初始大小
* @param nrec: 记录部分的初始大小
*/
int native_createtable(lua_State *L) {
    int narr = luaL_checkinteger(L, 1);
    int nrec = luaL_checkinteger(L, 2);
    lua_createtable(L, narr, nrec);
    return 1;
}

/**
* Lua底层API封装：创建新的用户数据
* @param sz: 大小
*/
int native_newuserdata(lua_State *L) {
    size_t sz = luaL_checkinteger(L, 1);
    void *ud = lua_newuserdata(L, sz);
    lua_pushlightuserdata(L, ud);
    return 1;
}

/**
* Lua底层API封装：获取元表
* @param objindex: 对象索引
* @return 元表或nil
*/
int native_getmetatable(lua_State *L) {
    int objindex = luaL_checkinteger(L, 1);
    int result = lua_getmetatable(L, objindex);
    return result;
}

/**
* Lua底层API封装：设置元表
* @param objindex: 对象索引
* @param metatable: 元表（从栈顶获取）
*/
int native_setmetatable(lua_State *L) {
    int objindex = luaL_checkinteger(L, 1);
    int result = lua_setmetatable(L, objindex);
    return result;
}

/**
* Lua底层API封装：获取全局变量
* @param name: 变量名
* @return 变量值
*/
int native_getglobal(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    lua_getglobal(L, name);
    return 1;
}

/**
* Lua底层API封装：设置全局变量
* @param name: 变量名
* @param value: 变量值（从栈顶获取）
*/
int native_setglobal(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    lua_setglobal(L, name);
    return 0;
}

/**
* Lua底层API封装：加载Lua代码块
* @param reader: 读取函数
* @param dt: 数据
* @param chunkname: 代码块名称
* @param mode: 模式
* @return 加载结果
*/
int native_load(lua_State *L) {
    lua_Reader reader = (lua_Reader)lua_touserdata(L, 1);
    void *dt = lua_touserdata(L, 2);
    const char *chunkname = luaL_checkstring(L, 3);
    const char *mode = luaL_optstring(L, 4, NULL);
    int result = lua_load(L, reader, dt, chunkname, mode);
    lua_pushinteger(L, result);
    return 1;
}

/**
* Lua底层API封装：保护模式下调用函数
* @param nargs: 参数数量
* @param nresults: 返回值数量
* @param errfunc: 错误处理函数索引
* @return 调用结果
*/
int native_pcall(lua_State *L) {
    int nargs = luaL_checkinteger(L, 1);
    int nresults = luaL_checkinteger(L, 2);
    int errfunc = luaL_optinteger(L, 3, 0);
    int result = lua_pcall(L, nargs, nresults, errfunc);
    lua_pushinteger(L, result);
    return 1;
}

/**
* Lua底层API封装：调用函数（无保护模式）
* @param nargs: 参数数量
* @param nresults: 返回值数量
*/
int native_call(lua_State *L) {
    int nargs = luaL_checkinteger(L, 1);
    int nresults = luaL_checkinteger(L, 2);
    lua_call(L, nargs, nresults);
    return 0;
}

/**
* Lua底层API封装：协程 - 让出执行权
* @param nresults: 返回值数量
* @return 让出结果
*/
int native_yield(lua_State *L) {
    int nresults = luaL_checkinteger(L, 1);
    int result = lua_yield(L, nresults);
    lua_pushinteger(L, result);
    return 1;
}

/**
* Lua底层API封装：协程 - 恢复执行
* @param from: 父线程
* @param narg: 参数数量
* @return 恢复结果
*/
int native_resume(lua_State *L) {
    lua_State *from = (lua_State *)lua_touserdata(L, 1);
    int narg = luaL_checkinteger(L, 2);
    int nres;
    int result = lua_resume(L, from, narg, &nres);
    lua_pushinteger(L, result);
    lua_pushinteger(L, nres);
    return 2;
}

/**
* Lua底层API封装：协程 - 获取状态
* @return 协程状态
*/
int native_status(lua_State *L) {
    int status = lua_status(L);
    lua_pushinteger(L, status);
    return 1;
}

/**
* Lua底层API封装：协程 - 检查是否可以让出
* @return 1 if can yield, 0 otherwise
*/
int native_isyieldable(lua_State *L) {
    int result = lua_isyieldable(L);
    lua_pushboolean(L, result);
    return 1;
}

/**
* Lua底层API封装：垃圾回收
* @param what: 操作类型
* @param data: 额外数据
* @return 操作结果
*/
int native_gc(lua_State *L) {
    int what = luaL_checkinteger(L, 1);
    int data = luaL_optinteger(L, 2, 0);
    int result = lua_gc(L, what, data);
    lua_pushinteger(L, result);
    return 1;
}

/**
* Lua底层API封装：连接字符串
* @param n: 要连接的字符串数量
*/
int native_concat(lua_State *L) {
    int n = luaL_checkinteger(L, 1);
    lua_concat(L, n);
    return 1;
}

/**
* Lua底层API封装：获取长度
* @param idx: 栈索引
* @return 长度
*/
int native_len(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    lua_len(L, idx);
    return 1;
}

/**
* Lua底层API封装：遍历表
* @param idx: 表的索引
* @return 下一个键值对或nil
*/
int native_next(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    int result = lua_next(L, idx);
    return result;
}

/**
* Lua辅助库封装：加载字符串
* @param s: Lua代码字符串
* @return 加载结果
*/
int nativeL_loadstring(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    int result = luaL_loadstring(L, s);
    lua_pushinteger(L, result);
    return 1;
}

/**
* Lua辅助库封装：加载文件
* @param filename: 文件名
* @return 加载结果
*/
int nativeL_loadfile(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    int result = luaL_loadfile(L, filename);
    lua_pushinteger(L, result);
    return 1;
}

/**
* Lua辅助库封装：加载并执行文件
* @param filename: 文件名
* @return 执行结果
*/
int nativeL_dofile(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    int result = luaL_dofile(L, filename);
    lua_pushinteger(L, result);
    return 1;
}

/**
* Lua辅助库封装：加载并执行文件，如果存在main函数则调用它
* @param filename: 文件名
* @return 执行结果（成功返回LUA_OK，失败返回错误码）
*/
int nativeL_dofile_with_main(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    
    int load_result = luaL_loadfile(L, filename);
    if (load_result != LUA_OK) {
        return load_result;
    }
    
    int exec_result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (exec_result != LUA_OK) {
        return exec_result;
    }
    
    lua_getglobal(L, "main");
    if (lua_type(L, -1) == LUA_TFUNCTION) {
        lua_remove(L, -2);
        int call_result = lua_pcall(L, 0, LUA_MULTRET, 0);
        return call_result;
    }
    
    lua_pop(L, 1);
    return LUA_OK;
}

/**
* Lua辅助库封装：加载并执行字符串
* @param s: Lua代码字符串
* @return 执行结果
*/
int nativeL_dostring(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    int result = luaL_dostring(L, s);
    lua_pushinteger(L, result);
    return 1;
}

/**
* Lua辅助库封装：创建新库
* @param l: 库函数列表
*/
int nativeL_newlib(lua_State *L) {
    luaL_Reg *l = (luaL_Reg *)lua_touserdata(L, 1);
    int size = 0;
    while (l[size].name != NULL) size++;
    lua_createtable(L, 0, size);
    luaL_setfuncs(L, l, 0);
    return 1;
}

/**
* Lua辅助库封装：设置库函数
* @param l: 库函数列表
* @param nup: 上值数量
*/
int nativeL_setfuncs(lua_State *L) {
    luaL_Reg *l = (luaL_Reg *)lua_touserdata(L, 1);
    int nup = luaL_checkinteger(L, 2);
    luaL_setfuncs(L, l, nup);
    return 0;
}

/**
* Lua辅助库封装：获取命名元表
* @param tname: 元表名称
* @return 元表
*/
int nativeL_getmetatable(lua_State *L) {
    const char *tname = luaL_checkstring(L, 1);
    luaL_getmetatable(L, tname);
    return 1;
}

/**
* Lua辅助库封装：设置命名元表
* @param tname: 元表名称
*/
int nativeL_setmetatable(lua_State *L) {
    const char *tname = luaL_checkstring(L, 1);
    luaL_setmetatable(L, tname);
    return 0;
}

/**
* Lua辅助库封装：检查字符串
* @param arg: 参数索引
* @return 字符串
*/
int nativeL_checkstring(lua_State *L) {
    int arg = luaL_checkinteger(L, 1);
    const char *s = luaL_checkstring(L, arg);
    lua_pushstring(L, s);
    return 1;
}

/**
* Lua辅助库封装：检查数字
* @param arg: 参数索引
* @return 数字
*/
int nativeL_checknumber(lua_State *L) {
    int arg = luaL_checkinteger(L, 1);
    lua_Number n = luaL_checknumber(L, arg);
    lua_pushnumber(L, n);
    return 1;
}

/**
* Lua辅助库封装：检查整数
* @param arg: 参数索引
* @return 整数
*/
int nativeL_checkinteger(lua_State *L) {
    int arg = luaL_checkinteger(L, 1);
    lua_Integer n = luaL_checkinteger(L, arg);
    lua_pushinteger(L, n);
    return 1;
}

/**
* Lua辅助库封装：抛出错误
* @param fmt: 错误格式字符串
* @param ...: 格式化参数
*/
int nativeL_error(lua_State *L) {
    const char *fmt = luaL_checkstring(L, 1);
    luaL_error(L, fmt);
    return 0;
}

/**
* 更底层的VM操作：获取内存分配函数
* @return 内存分配函数和用户数据
*/
int native_getallocf(lua_State *L) {
    void *ud;
    lua_Alloc f = lua_getallocf(L, &ud);
    lua_pushlightuserdata(L, f);
    lua_pushlightuserdata(L, ud);
    return 2;
}

/**
* 更底层的VM操作：设置内存分配函数
* @param f: 内存分配函数
* @param ud: 用户数据
*/
int native_setallocf(lua_State *L) {
    lua_Alloc f = (lua_Alloc)lua_touserdata(L, 1);
    void *ud = lua_touserdata(L, 2);
    lua_setallocf(L, f, ud);
    return 0;
}

/**
* 更底层的VM操作：设置或获取panic处理函数
* @param f: 新的panic处理函数（可选）
* @return 旧的panic处理函数
*/
int native_atpanic(lua_State *L) {
    if (lua_isnoneornil(L, 1)) {
        lua_CFunction old = lua_atpanic(L, NULL);
        lua_pushlightuserdata(L, old);
    } else {
        lua_CFunction f = (lua_CFunction)lua_touserdata(L, 1);
        lua_CFunction old = lua_atpanic(L, f);
        lua_pushlightuserdata(L, old);
    }
    return 1;
}

/**
* 更底层的VM操作：检查栈空间
* @param n: 需要的额外栈空间
* @return 1 if successful, 0 otherwise
*/
int native_checkstack(lua_State *L) {
    int n = luaL_checkinteger(L, 1);
    int result = lua_checkstack(L, n);
    lua_pushboolean(L, result);
    return 1;
}

/**
* 更底层的VM操作：在状态机之间移动值
* @param to: 目标状态机
* @param n: 要移动的值的数量
*/
int native_xmove(lua_State *L) {
    lua_State *to = (lua_State *)lua_touserdata(L, 1);
    int n = luaL_checkinteger(L, 2);
    lua_xmove(L, to, n);
    return 0;
}

/**
* 更底层的VM操作：将函数转储为二进制形式
* @param writer: 写入函数
* @param data: 写入函数的用户数据
* @param strip: 是否剥离调试信息
* @return 转储结果
*/
int native_dump(lua_State *L) {
    lua_Writer writer = (lua_Writer)lua_touserdata(L, 1);
    void *data = lua_touserdata(L, 2);
    int strip = lua_toboolean(L, 3);
    int result = lua_dump(L, writer, data, strip);
    lua_pushinteger(L, result);
    return 1;
}

/**
* 更底层的VM操作：字符串转数字
* @param s: 字符串
* @return 转换是否成功
*/
int native_stringtonumber(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    size_t result = lua_stringtonumber(L, s);
    lua_pushinteger(L, result);
    return 1;
}

/**
* 更底层的VM操作：数字转字符串
* @param idx: 数字在栈中的索引
* @param buff: 缓冲区（可选）
* @return 转换后的字符串长度
*/
int native_numbertocstring(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    char buff[64];
    unsigned result = lua_numbertocstring(L, idx, buff);
    lua_pushinteger(L, result);
    lua_pushstring(L, buff);
    return 2;
}

/**
* 更底层的VM操作：停止垃圾回收
*/
int native_gcstop(lua_State *L) {
    lua_gc(L, LUA_GCSTOP, 0);
    return 0;
}

/**
* 更底层的VM操作：重启垃圾回收
*/
int native_gcrestart(lua_State *L) {
    lua_gc(L, LUA_GCRESTART, 0);
    return 0;
}

/**
* 更底层的VM操作：获取当前垃圾回收计数
* @return 当前垃圾回收计数
*/
int native_gccount(lua_State *L) {
    int result = lua_gc(L, LUA_GCCOUNT, 0);
    lua_pushinteger(L, result);
    return 1;
}

/**
* 更底层的VM操作：获取当前垃圾回收计数的小数部分
* @return 当前垃圾回收计数的小数部分
*/
int native_gccountb(lua_State *L) {
    int result = lua_gc(L, LUA_GCCOUNTB, 0);
    lua_pushinteger(L, result);
    return 1;
}

/**
* 更底层的VM操作：执行一步垃圾回收
* @param step: 步长
* @return 垃圾回收是否完成
*/
int native_gcstep(lua_State *L) {
    int step = luaL_checkinteger(L, 1);
    int result = lua_gc(L, LUA_GCSTEP, step);
    lua_pushboolean(L, result);
    return 1;
}

/**
* 更底层的VM操作：获取VM状态
* @return VM状态信息
*/
int native_getvmstate(lua_State *L) {
    // 获取Lua状态机的一些内部状态信息
    lua_pushinteger(L, L->status);
    lua_pushboolean(L, L->hook != NULL);
    return 2;
}

/**
* 更底层的VM操作：获取VM内存使用情况
* @return 内存使用信息
*/
int native_getvmmemory(lua_State *L) {
    // 注意：这是一个示例，实际实现可能需要访问Lua内部结构
    int count = lua_gc(L, LUA_GCCOUNT, 0);
    int countb = lua_gc(L, LUA_GCCOUNTB, 0);
    lua_pushinteger(L, count);
    lua_pushinteger(L, countb);
    return 2;
}

/**
* 更底层的VM操作：获取VM对象统计
* @return 对象统计信息
*/
int native_getvmobjects(lua_State *L) {
    // 注意：这是一个示例，实际实现可能需要访问Lua内部结构
    // 这里返回一些简单的统计信息
    lua_pushinteger(L, lua_gettop(L));
    return 1;
}

/**
* 更底层的VM操作：获取注册表
* @return 注册表
*/
int native_getvmregistry(lua_State *L) {
    lua_pushvalue(L, LUA_REGISTRYINDEX);
    return 1;
}

/**
* 更底层的VM操作：获取当前环境
* @return 当前环境
*/
int native_getvmenv(lua_State *L) {
    lua_pushglobaltable(L);
    return 1;
}

/**
* 更底层的VM操作：获取调用栈信息
* @param level: 栈级别
* @return 栈信息
*/
int native_getstack(lua_State *L) {
    int level = luaL_checkinteger(L, 1);
    lua_Debug ar;
    int result = lua_getstack(L, level, &ar);
    lua_pushboolean(L, result);
    return 1;
}

/**
* 更底层的VM操作：获取调用信息
* @param what: 要获取的信息类型
* @return 调用信息
*/
int native_getinfo(lua_State *L) {
    const char *what = luaL_checkstring(L, 1);
    lua_Debug ar;
    int result = lua_getinfo(L, what, &ar);
    if (result) {
        lua_newtable(L);
        lua_pushstring(L, "source");
        lua_pushstring(L, ar.source);
        lua_settable(L, -3);
        lua_pushstring(L, "currentline");
        lua_pushinteger(L, ar.currentline);
        lua_settable(L, -3);
        lua_pushstring(L, "what");
        lua_pushstring(L, ar.what);
        lua_settable(L, -3);
        lua_pushstring(L, "name");
        lua_pushstring(L, ar.name);
        lua_settable(L, -3);
        lua_pushstring(L, "namewhat");
        lua_pushstring(L, ar.namewhat);
        lua_settable(L, -3);
        return 1;
    } else {
        lua_pushnil(L);
        return 1;
    }
}

/**
* 更底层的VM操作：设置调试钩子
* @param func: 钩子函数
* @param mask: 钩子掩码
* @param count: 钩子计数
*/
int native_sethook(lua_State *L) {
    lua_Hook func = (lua_Hook)lua_touserdata(L, 1);
    int mask = luaL_checkinteger(L, 2);
    int count = luaL_checkinteger(L, 3);
    lua_sethook(L, func, mask, count);
    return 0;
}

/**
* 更底层的VM操作：获取调试钩子
* @return 钩子函数
*/
int native_gethook(lua_State *L) {
    lua_Hook func = lua_gethook(L);
    lua_pushlightuserdata(L, func);
    return 1;
}

/**
* 更底层的VM操作：获取调试钩子掩码
* @return 钩子掩码
*/
int native_gethookmask(lua_State *L) {
    int mask = lua_gethookmask(L);
    lua_pushinteger(L, mask);
    return 1;
}

/**
* 更底层的VM操作：获取调试钩子计数
* @return 钩子计数
*/
int native_gethookcount(lua_State *L) {
    int count = lua_gethookcount(L);
    lua_pushinteger(L, count);
    return 1;
}

/**
* 更底层的VM操作：创建指令
* @param op: 操作码
* @param a: A参数
* @param b: B参数
* @param c: C参数
* @param k: k标志
* @return 创建的指令
*/
int native_create_instruction(lua_State *L) {
    int op = luaL_checkinteger(L, 1);
    int a = luaL_checkinteger(L, 2);
    int b = luaL_optinteger(L, 3, 0);
    int c = luaL_optinteger(L, 4, 0);
    int k = luaL_optinteger(L, 5, 0);
    
    Instruction instr = CREATE_ABCk(op, a, b, c, k);
    lua_pushinteger(L, instr);
    return 1;
}

/**
* 更底层的VM操作：获取指令的操作码
* @param instr: 指令
* @return 操作码
*/
int native_get_opcode(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int opcode = GET_OPCODE(instr);
    lua_pushinteger(L, opcode);
    return 1;
}

/**
* 更底层的VM操作：设置指令的操作码
* @param instr: 指令
* @param op: 新的操作码
* @return 修改后的指令
*/
int native_set_opcode(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int op = luaL_checkinteger(L, 2);
    SET_OPCODE(instr, op);
    lua_pushinteger(L, instr);
    return 1;
}

/**
* 更底层的VM操作：获取指令的A参数
* @param instr: 指令
* @return A参数
*/
int native_getarg_a(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int a = GETARG_A(instr);
    lua_pushinteger(L, a);
    return 1;
}

/**
* 更底层的VM操作：设置指令的A参数
* @param instr: 指令
* @param a: 新的A参数
* @return 修改后的指令
*/
int native_setarg_a(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int a = luaL_checkinteger(L, 2);
    SETARG_A(instr, a);
    lua_pushinteger(L, instr);
    return 1;
}

/**
* 更底层的VM操作：获取指令的B参数
* @param instr: 指令
* @return B参数
*/
int native_getarg_b(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int b = GETARG_B(instr);
    lua_pushinteger(L, b);
    return 1;
}

/**
* 更底层的VM操作：设置指令的B参数
* @param instr: 指令
* @param b: 新的B参数
* @return 修改后的指令
*/
int native_setarg_b(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int b = luaL_checkinteger(L, 2);
    SETARG_B(instr, b);
    lua_pushinteger(L, instr);
    return 1;
}

/**
* 更底层的VM操作：获取指令的C参数
* @param instr: 指令
* @return C参数
*/
int native_getarg_c(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int c = GETARG_C(instr);
    lua_pushinteger(L, c);
    return 1;
}

/**
* 更底层的VM操作：设置指令的C参数
* @param instr: 指令
* @param c: 新的C参数
* @return 修改后的指令
*/
int native_setarg_c(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int c = luaL_checkinteger(L, 2);
    SETARG_C(instr, c);
    lua_pushinteger(L, instr);
    return 1;
}

/**
* 更底层的VM操作：获取指令的Bx参数
* @param instr: 指令
* @return Bx参数
*/
int native_getarg_bx(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int bx = GETARG_Bx(instr);
    lua_pushinteger(L, bx);
    return 1;
}

/**
* 更底层的VM操作：设置指令的Bx参数
* @param instr: 指令
* @param bx: 新的Bx参数
* @return 修改后的指令
*/
int native_setarg_bx(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int bx = luaL_checkinteger(L, 2);
    SETARG_Bx(instr, bx);
    lua_pushinteger(L, instr);
    return 1;
}

/**
* 更底层的VM操作：获取指令的sBx参数
* @param instr: 指令
* @return sBx参数
*/
int native_getarg_sbx(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int sbx = GETARG_sBx(instr);
    lua_pushinteger(L, sbx);
    return 1;
}

/**
* 更底层的VM操作：设置指令的sBx参数
* @param instr: 指令
* @param sbx: 新的sBx参数
* @return 修改后的指令
*/
int native_setarg_sbx(lua_State *L) {
    Instruction instr = (Instruction)luaL_checkinteger(L, 1);
    int sbx = luaL_checkinteger(L, 2);
    SETARG_sBx(instr, sbx);
    lua_pushinteger(L, instr);
    return 1;
}

/**
* 更底层的VM操作：执行指令
* 注意：这是一个非常危险的操作，可能会破坏VM状态
* @param instr: 要执行的指令
*/
int native_execute_instruction(lua_State *L) {
    // 注意：直接执行单个指令是非常复杂和危险的，需要完整的VM上下文
    // 这里我们只实现一个简单的模拟，实际使用时需要小心
    lua_pushstring(L, "execute_instruction is not implemented due to its complexity and danger");
    return 1;
}

/**
* 将Lua代码转换为指令列表
* @param code: Lua代码字符串
* @return 指令列表（整数数组）
*/
int native_code_to_instructions(lua_State *L) {
    const char *code = luaL_checkstring(L, 1);
    const char *chunkname = luaL_optstring(L, 2, "<native_code>");
    
    // 1. 加载并编译Lua代码
    int status = luaL_loadstring(L, code);
    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        lua_pushnil(L);
        lua_pushstring(L, err);
        return 2; // 返回nil和错误信息
    }
    
    // 2. 检查是否为Lua闭包
    if (!ttisLclosure(s2v(L->top.p - 1))) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to get function prototype: not a Lua closure");
        lua_remove(L, -3); // 移除加载的函数
        return 2;
    }
    
    // 3. 获取函数原型
    LClosure *cl = clLvalue(s2v(L->top.p - 1));
    const Proto *p = cl->p;
    if (p == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to get function prototype: prototype is NULL");
        lua_remove(L, -3); // 移除加载的函数
        return 2;
    }
    
    // 4. 创建指令列表数组
    lua_newtable(L);
    
    // 5. 填充指令列表
    for (int i = 0; i < p->sizecode; i++) {
        lua_pushinteger(L, i + 1); // 索引从1开始
        lua_pushinteger(L, p->code[i]); // 存储指令
        lua_settable(L, -3);
    }
    
    // 6. 移除加载的函数，返回指令列表
    lua_remove(L, -2);
    return 1;
}

/**
* 从指令列表加载函数
* @param instructions: 指令列表（整数数组）
* @return 加载的函数
*/
int native_load_instructions(lua_State *L) {
    // 检查参数是否为表
    if (!lua_istable(L, 1)) {
        return luaL_error(L, "Expected a table of instructions");
    }
    
    // 1. 获取指令列表的长度
    int n = (int)lua_rawlen(L, 1);
    if (n == 0) {
        return luaL_error(L, "Instruction list is empty");
    }
    
    // 2. 创建一个简单的Lua函数，用于执行指令
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    
    // 3. 构建一个函数，该函数将使用我们的指令
    luaL_addstring(&b, "return function()\n");
    luaL_addstring(&b, "  -- This function will execute custom instructions\n");
    luaL_addstring(&b, "  -- For demonstration, we'll just return the instruction list\n");
    luaL_addstring(&b, "  local instructions = {...}\n");
    luaL_addstring(&b, "  return instructions\n");
    luaL_addstring(&b, "end\n");
    luaL_pushresult(&b);
    
    // 4. 加载这个函数
    int status = luaL_loadstring(L, lua_tostring(L, -1));
    lua_remove(L, -2); // 移除字符串
    
    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        return luaL_error(L, "Failed to create function: %s", err);
    }
    
    // 5. 执行这个函数，获取内部函数
    status = lua_pcall(L, 0, 1, 0);
    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        return luaL_error(L, "Failed to execute function: %s", err);
    }
    
    // 6. 现在我们有了一个函数，我们可以修改它的原型来包含自定义指令
    // 但直接修改原型是危险的，所以我们返回函数和指令列表
    // 这样Lua代码可以根据需要处理它们
    
    return 1;
}

/**
* 执行从指令创建的函数
* @param func: 从指令创建的函数
* @param ...: 函数参数
* @return 函数返回值
*/
int native_exec_instructions(lua_State *L) {
    // 检查参数是否为函数
    if (!lua_isfunction(L, 1)) {
        return luaL_error(L, "Expected a function");
    }
    
    // 获取参数数量
    int nargs = lua_gettop(L) - 1;
    
    // 使用pcall执行函数
    int status = lua_pcall(L, nargs, LUA_MULTRET, 0);
    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        lua_pushnil(L);
        lua_pushstring(L, err);
        return 2; // 返回nil和错误信息
    }
    
    // 返回函数的返回值
    return lua_gettop(L);
}
