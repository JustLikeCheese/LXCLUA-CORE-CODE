#include "utils.h"

/**
* 设置模块信息
* @param L: Lua状态机
*/
void utils_set_info(lua_State *L) {
    lua_pushliteral(L, "_COPYRIGHT");
    lua_pushliteral(L, "Copyright (C) 2026 DifierLine");
    lua_settable(L, -3);
    
    lua_pushliteral(L, "_DESCRIPTION");
    lua_pushliteral(L, "Native module for Lua");
    lua_settable(L, -3);
    
    lua_pushliteral(L, "_VERSION");
    lua_pushliteral(L, "1.0.0");
    lua_settable(L, -3);
}

/**
* 获取模块名称
* @return 模块名称字符串
*/
const char *utils_get_module_name(void) {
    return "native";
}
