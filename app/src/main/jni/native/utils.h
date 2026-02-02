#ifndef UTILS_H
#define UTILS_H

#include <lua.h>
#include <lauxlib.h>

/**
* 工具函数声明
*/
void utils_set_info(lua_State *L);
const char *utils_get_module_name(void);

#endif /* UTILS_H */
