#include "native.h"
#include "utils.h"

/**
* 模块函数列表
*/
static const struct luaL_Reg native_lib[] = {
    // 原生功能函数
    {"get_current_time", native_get_current_time},
    {"add", native_add},
    {"log", native_log},
    {"get_version", native_get_version},
    {"reverse_string", native_reverse_string},
    
    // Lua底层API封装 - 基本操作
    {"gettop", native_gettop},
    {"settop", native_settop},
    {"pushnil", native_pushnil},
    {"pushboolean", native_pushboolean},
    {"pushinteger", native_pushinteger},
    {"pushnumber", native_pushnumber},
    {"pushstring", native_pushstring},
    {"pushlstring", native_pushlstring},
    {"pushcclosure", native_pushcclosure},
    {"pushlightuserdata", native_pushlightuserdata},
    {"pushthread", native_pushthread},
    
    // Lua底层API封装 - 访问函数
    {"type", native_type},
    {"typename", native_typename},
    {"tonumber", native_tonumber},
    {"tointeger", native_tointeger},
    {"toboolean", native_toboolean},
    {"tostring", native_tostring},
    {"rawlen", native_rawlen},
    {"touserdata", native_touserdata},
    {"tothread", native_tothread},
    
    // Lua底层API封装 - 比较和算术
    {"rawequal", native_rawequal},
    {"compare", native_compare},
    {"arith", native_arith},
    
    // Lua底层API封装 - 表操作
    {"gettable", native_gettable},
    {"getfield", native_getfield},
    {"geti", native_geti},
    {"rawget", native_rawget},
    {"rawgeti", native_rawgeti},
    {"settable", native_settable},
    {"setfield", native_setfield},
    {"seti", native_seti},
    {"rawset", native_rawset},
    {"rawseti", native_rawseti},
    {"createtable", native_createtable},
    {"newuserdata", native_newuserdata},
    {"getmetatable", native_getmetatable},
    {"setmetatable", native_setmetatable},
    
    // Lua底层API封装 - 全局变量
    {"getglobal", native_getglobal},
    {"setglobal", native_setglobal},
    
    // Lua底层API封装 - 加载和调用
    {"load", native_load},
    {"pcall", native_pcall},
    {"call", native_call},
    
    // Lua底层API封装 - 协程
    {"yield", native_yield},
    {"resume", native_resume},
    {"status", native_status},
    {"isyieldable", native_isyieldable},
    
    // Lua底层API封装 - 垃圾回收
    {"gc", native_gc},
    
    // Lua底层API封装 - 其他
    {"concat", native_concat},
    {"len", native_len},
    {"next", native_next},
    
    // Lua辅助库封装
    {"L_loadstring", nativeL_loadstring},
    {"L_loadfile", nativeL_loadfile},
    {"L_dofile", nativeL_dofile},
    {"L_dofile_with_main", nativeL_dofile_with_main},
    {"L_dostring", nativeL_dostring},
    {"L_newlib", nativeL_newlib},
    {"L_setfuncs", nativeL_setfuncs},
    {"L_getmetatable", nativeL_getmetatable},
    {"L_setmetatable", nativeL_setmetatable},
    {"L_checkstring", nativeL_checkstring},
    {"L_checknumber", nativeL_checknumber},
    {"L_checkinteger", nativeL_checkinteger},
    {"L_error", nativeL_error},
    
    // 更底层的VM操作 - 内存管理
    {"getallocf", native_getallocf},
    {"setallocf", native_setallocf},
    
    // 更底层的VM操作 - 错误处理
    {"atpanic", native_atpanic},
    
    // 更底层的VM操作 - 栈管理
    {"checkstack", native_checkstack},
    {"xmove", native_xmove},
    
    // 更底层的VM操作 - 代码管理
    {"dump", native_dump},
    
    // 更底层的VM操作 - 字符串转换
    {"stringtonumber", native_stringtonumber},
    {"numbertocstring", native_numbertocstring},
    
    // 更底层的VM操作 - 垃圾回收高级操作
    {"gcstop", native_gcstop},
    {"gcrestart", native_gcrestart},
    {"gccount", native_gccount},
    {"gccountb", native_gccountb},
    {"gcstep", native_gcstep},
    
    // 内部VM状态操作
    {"getvmstate", native_getvmstate},
    {"getvmmemory", native_getvmmemory},
    {"getvmobjects", native_getvmobjects},
    {"getvmregistry", native_getvmregistry},
    {"getvmenv", native_getvmenv},
    
    // 调试相关操作
    {"getstack", native_getstack},
    {"getinfo", native_getinfo},
    {"sethook", native_sethook},
    {"gethook", native_gethook},
    {"gethookmask", native_gethookmask},
    {"gethookcount", native_gethookcount},
    
    // 更底层的VM操作 - 指令操作
    {"create_instruction", native_create_instruction},
    {"get_opcode", native_get_opcode},
    {"set_opcode", native_set_opcode},
    {"getarg_a", native_getarg_a},
    {"setarg_a", native_setarg_a},
    {"getarg_b", native_getarg_b},
    {"setarg_b", native_setarg_b},
    {"getarg_c", native_getarg_c},
    {"setarg_c", native_setarg_c},
    {"getarg_bx", native_getarg_bx},
    {"setarg_bx", native_setarg_bx},
    {"getarg_sbx", native_getarg_sbx},
    {"setarg_sbx", native_setarg_sbx},
    {"execute_instruction", native_execute_instruction},
    
    // 代码转指令和加载执行相关函数
    {"code_to_instructions", native_code_to_instructions},
    {"load_instructions", native_load_instructions},
    {"exec_instructions", native_exec_instructions},
    
    {NULL, NULL} // 结束标记
};

/**
* 模块初始化函数
* 当Lua调用require("native")时，会调用此函数
* @param L: Lua状态机
* @return 返回值数量
*/
int luaopen_native(lua_State *L) {
    // 创建一个新的库表
    luaL_newlib(L, native_lib);
    
    // 设置模块信息
    utils_set_info(L);
    
    return 1;
}
