#ifndef NATIVE_H
#define NATIVE_H

#include <lua.h>
#include <lauxlib.h>
#include "../lua/lstate.h"  // 添加Lua内部头文件，用于更底层的VM操作
#include "../lua/lgc.h"     // 垃圾回收相关
#include "../lua/lmem.h"    // 内存相关
#include "../lua/lobject.h"  // 对象相关

/**
* 模块初始化函数声明
*/
int luaopen_native(lua_State *L);

/**
* 原生功能相关函数声明
*/
int native_get_current_time(lua_State *L);
int native_add(lua_State *L);
int native_log(lua_State *L);
int native_get_version(lua_State *L);
int native_reverse_string(lua_State *L);

/**
* Lua底层API封装函数声明 - 基本操作
*/
int native_gettop(lua_State *L);
int native_settop(lua_State *L);
int native_pushnil(lua_State *L);
int native_pushboolean(lua_State *L);
int native_pushinteger(lua_State *L);
int native_pushnumber(lua_State *L);
int native_pushstring(lua_State *L);
int native_pushlstring(lua_State *L);
int native_pushcclosure(lua_State *L);
int native_pushlightuserdata(lua_State *L);
int native_pushthread(lua_State *L);

/**
* Lua底层API封装函数声明 - 访问函数
*/
int native_type(lua_State *L);
int native_typename(lua_State *L);
int native_tonumber(lua_State *L);
int native_tointeger(lua_State *L);
int native_toboolean(lua_State *L);
int native_tostring(lua_State *L);
int native_rawlen(lua_State *L);
int native_touserdata(lua_State *L);
int native_tothread(lua_State *L);

/**
* Lua底层API封装函数声明 - 比较和算术
*/
int native_rawequal(lua_State *L);
int native_compare(lua_State *L);
int native_arith(lua_State *L);

/**
* Lua底层API封装函数声明 - 表操作
*/
int native_gettable(lua_State *L);
int native_getfield(lua_State *L);
int native_geti(lua_State *L);
int native_rawget(lua_State *L);
int native_rawgeti(lua_State *L);
int native_settable(lua_State *L);
int native_setfield(lua_State *L);
int native_seti(lua_State *L);
int native_rawset(lua_State *L);
int native_rawseti(lua_State *L);
int native_createtable(lua_State *L);
int native_newuserdata(lua_State *L);
int native_getmetatable(lua_State *L);
int native_setmetatable(lua_State *L);

/**
* Lua底层API封装函数声明 - 全局变量
*/
int native_getglobal(lua_State *L);
int native_setglobal(lua_State *L);

/**
* Lua底层API封装函数声明 - 加载和调用
*/
int native_load(lua_State *L);
int native_pcall(lua_State *L);
int native_call(lua_State *L);

/**
* Lua底层API封装函数声明 - 协程
*/
int native_yield(lua_State *L);
int native_resume(lua_State *L);
int native_status(lua_State *L);
int native_isyieldable(lua_State *L);

/**
* Lua底层API封装函数声明 - 垃圾回收
*/
int native_gc(lua_State *L);

/**
* Lua底层API封装函数声明 - 其他
*/
int native_concat(lua_State *L);
int native_len(lua_State *L);
int native_next(lua_State *L);

/**
* Lua底层API封装函数声明 - 辅助库
*/
int nativeL_loadstring(lua_State *L);
int nativeL_loadfile(lua_State *L);
int nativeL_dofile(lua_State *L);
int nativeL_dofile_with_main(lua_State *L);
int nativeL_dostring(lua_State *L);
int nativeL_newlib(lua_State *L);
int nativeL_setfuncs(lua_State *L);
int nativeL_getmetatable(lua_State *L);
int nativeL_setmetatable(lua_State *L);
int nativeL_checkstring(lua_State *L);
int nativeL_checknumber(lua_State *L);
int nativeL_checkinteger(lua_State *L);
int nativeL_error(lua_State *L);

/**
* 更底层的VM操作函数声明 - 内存管理
*/
int native_getallocf(lua_State *L);
int native_setallocf(lua_State *L);

/**
* 更底层的VM操作函数声明 - 错误处理
*/
int native_atpanic(lua_State *L);

/**
* 更底层的VM操作函数声明 - 栈管理
*/
int native_checkstack(lua_State *L);
int native_xmove(lua_State *L);

/**
* 更底层的VM操作函数声明 - 代码管理
*/
int native_dump(lua_State *L);

/**
* 更底层的VM操作函数声明 - 字符串转换
*/
int native_stringtonumber(lua_State *L);
int native_numbertocstring(lua_State *L);

/**
* 更底层的VM操作函数声明 - 垃圾回收高级操作
*/
int native_gcstop(lua_State *L);
int native_gcrestart(lua_State *L);
int native_gccount(lua_State *L);
int native_gccountb(lua_State *L);
int native_gcstep(lua_State *L);

/**
* 内部VM状态操作函数声明
*/
int native_getvmstate(lua_State *L);
int native_getvmmemory(lua_State *L);
int native_getvmobjects(lua_State *L);
int native_getvmregistry(lua_State *L);
int native_getvmenv(lua_State *L);

/**
* 调试相关函数声明
*/
int native_getstack(lua_State *L);
int native_getinfo(lua_State *L);
int native_sethook(lua_State *L);
int native_gethook(lua_State *L);
int native_gethookmask(lua_State *L);
int native_gethookcount(lua_State *L);

/**
* 更底层的VM操作函数声明 - 指令操作
*/
int native_create_instruction(lua_State *L);
int native_get_opcode(lua_State *L);
int native_set_opcode(lua_State *L);
int native_getarg_a(lua_State *L);
int native_setarg_a(lua_State *L);
int native_getarg_b(lua_State *L);
int native_setarg_b(lua_State *L);
int native_getarg_c(lua_State *L);
int native_setarg_c(lua_State *L);
int native_getarg_bx(lua_State *L);
int native_setarg_bx(lua_State *L);
int native_getarg_sbx(lua_State *L);
int native_setarg_sbx(lua_State *L);
int native_execute_instruction(lua_State *L);

/**
* 代码转指令和加载执行相关函数声明
*/
int native_code_to_instructions(lua_State *L);
int native_load_instructions(lua_State *L);
int native_exec_instructions(lua_State *L);

#endif /* NATIVE_H */
