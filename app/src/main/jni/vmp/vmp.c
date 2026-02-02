#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "vmp.h"
#include "vmp_proto.h"

struct VMP_Env {
    lua_State *L;
    int debug_mode;
    int proto_share_enabled;
    VMPProtoRegistry *proto_reg;
    void *owner;
};

/**
 * @brief 身份函数，返回第一个参数
 * @param L Lua状态机
 * @return 1
 */
static int vmp_identity(lua_State *L) {
    lua_settop(L, 1);
    return 1;
}

/**
 * @brief 递归复制值从一个Lua状态到另一个
 * @param from 源Lua状态机
 * @param to 目标Lua状态机
 * @param depth 当前递归深度
 * @return 1 表示成功，0 表示失败
 */
static int copy_value(lua_State *from, lua_State *to, int depth) {
    if (depth > 10) {  // 防止无限递归
        lua_pushnil(to);
        return 0;
    }
    
    int type = lua_type(from, -1);
    switch (type) {
        case LUA_TNIL:
            lua_pushnil(to);
            break;
            
        case LUA_TBOOLEAN:
            lua_pushboolean(to, lua_toboolean(from, -1));
            break;
            
        case LUA_TNUMBER:
            if (lua_isinteger(from, -1)) {
                lua_pushinteger(to, lua_tointeger(from, -1));
            } else {
                lua_pushnumber(to, lua_tonumber(from, -1));
            }
            break;
            
        case LUA_TSTRING:
            lua_pushstring(to, lua_tostring(from, -1));
            break;
            
        case LUA_TTABLE:
            lua_newtable(to);
            
            // 复制元表
            if (lua_getmetatable(from, -1)) {
                copy_value(from, to, depth + 1);
                lua_setmetatable(to, -2);
            }
            
            // 遍历表并复制内容
            lua_pushnil(from);
            while (lua_next(from, -2) != 0) {
                // 复制键
                copy_value(from, to, depth + 1);
                // 复制值
                copy_value(from, to, depth + 1);
                // 设置到新表
                lua_settable(to, -3);
            }
            break;
            
        case LUA_TFUNCTION:
            // 只支持复制C函数
            if (lua_iscfunction(from, -1)) {
                lua_pushcfunction(to, lua_tocfunction(from, -1));
            } else {
                // 返回字符串说明这是一个C层函数，环境不同无法复制
                lua_pushstring(to, "[C层: 环境不同无法复制]");
            }
            break;
            
        case LUA_TUSERDATA:
        case LUA_TLIGHTUSERDATA:
            // 简单处理，将userdata转换为nil
             lua_pushstring(to, "[C层: 环境不同无法复制]");
            break;
            
        case LUA_TTHREAD:
            // 简单处理，将thread转换为nil
             lua_pushstring(to, "[C层: 环境不同无法复制]");
            break;
            
        default:
             lua_pushstring(to, "[C层: 环境不同无法复制]");
            break;
    }
    
    return 1;
}

/**
 * @brief 创建一个新的VMP虚拟环境（Lua模块API）
 * @param L Lua状态机
 * @return 新创建的VMP环境指针
 */
static int lua_vmp_create(lua_State *L) {
    VMP_Env *env = (VMP_Env *)lua_newuserdata(L, sizeof(VMP_Env));
    env->L = luaL_newstate();
    if (env->L == NULL) {
        return luaL_error(L, "无法创建VMP虚拟环境: 内存不足");
    }
    
    env->debug_mode = 0;
    env->proto_share_enabled = 0;
    env->proto_reg = NULL;
    env->owner = env;
    
    int open_libs = 1;
    
    if (lua_istable(L, 1)) {
        lua_getfield(L, 1, "open_libs");
        if (lua_isboolean(L, -1)) {
            open_libs = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);
        
        lua_getfield(L, 1, "debug_mode");
        if (lua_isboolean(L, -1)) {
            env->debug_mode = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);
        
        lua_getfield(L, 1, "share_proto");
        if (lua_isboolean(L, -1)) {
            env->proto_share_enabled = lua_toboolean(L, -1);
            if (env->proto_share_enabled) {
                env->proto_reg = vmp_proto_newregistry(lua_getallocf(env->L, NULL), NULL);
            }
        }
        lua_pop(L, 1);
    }
    
    if (open_libs) {
        luaL_openlibs(env->L);
    }
    
    luaL_getmetatable(L, "vmp_env");
    lua_setmetatable(L, -2);
    
    return 1;
}

/**
 * @brief 销毁VMP虚拟环境（Lua模块API）
 * @param L Lua状态机
 * @return 0
 */
static int lua_vmp_destroy(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    if (env->L) {
        if (env->proto_share_enabled && env->proto_reg) {
            vmp_proto_freeregistry(env->proto_reg);
            env->proto_reg = NULL;
        }
        lua_close(env->L);
        env->L = NULL;
    }
    return 0;
}

/**
 * @brief 在VMP虚拟环境中执行Lua代码（支持Proto共享）
 * @param env VMP环境
 * @param code Lua代码字符串
 * @return 执行状态
 */
static int vmp_load_and_exec(VMP_Env *env, const char *code) {
    int status;
    lua_State *L = env->L;
    int stack_top = lua_gettop(L);
    
    status = luaL_loadstring(L, code);
    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        if (env->debug_mode) {
            fprintf(stderr, "VMP加载错误: %s\n", err);
        }
        lua_settop(L, stack_top);
        return status;
    }
    
    int type_before = lua_type(L, -1);
    
    status = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (status != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        if (env->debug_mode) {
            fprintf(stderr, "VMP执行错误: %s\n", err);
        }
        lua_settop(L, stack_top);
        return status;
    }
    
    int result_count = lua_gettop(L) - stack_top;
    
    if (type_before == LUA_TFUNCTION && result_count == 0) {
        lua_pushstring(L, "[function defined]");
        result_count = 1;
    }
    
    return result_count;
}

/**
 * @brief 在VMP虚拟环境中执行Lua代码
 * @param L Lua状态机
 * @return 执行结果的数量
 */
static int lua_vmp_exec(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    const char *code = luaL_checkstring(L, 2);
    
    int result_count = vmp_load_and_exec(env, code);
    
    if (result_count < 0) {
        const char *err = lua_tostring(env->L, -1);
        if (env->debug_mode) {
            fprintf(stderr, "VMP执行错误: %s\n", err);
        }
        lua_pushstring(L, err);
        lua_pop(env->L, 1);
        return luaL_error(L, "VMP执行错误: %s", err);
    }
    
    int env_stack_top = lua_gettop(env->L) - result_count;
    for (int i = 0; i < result_count; i++) {
        lua_pushvalue(env->L, env_stack_top + i + 1);
        copy_value(env->L, L, 0);
    }
    lua_settop(env->L, env_stack_top);
    
    return result_count;
}

static int lua_vmp_set_global(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    const char *name = luaL_checkstring(L, 2);
    
    lua_pushvalue(L, 3);
    copy_value(L, env->L, 0);
    lua_setglobal(env->L, name);
    
    return 0;
}

static int lua_vmp_import_module(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    const char *module_name = luaL_checkstring(L, 2);
    
    lua_getglobal(L, "require");
    lua_pushstring(L, module_name);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        if (env->debug_mode) {
            fprintf(stderr, "VMP导入模块错误: %s\n", err);
        }
        return luaL_error(L, "VMP导入模块错误: %s", err);
    }
    
    copy_value(L, env->L, 0);
    lua_setglobal(env->L, module_name);
    
    lua_pop(L, 1);
    
    return 0;
}

static int lua_vmp_require(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    const char *module_name = luaL_checkstring(L, 2);
    
    lua_getglobal(env->L, "require");
    lua_pushstring(env->L, module_name);
    if (lua_pcall(env->L, 1, 1, 0) != LUA_OK) {
        const char *err = lua_tostring(env->L, -1);
        if (env->debug_mode) {
            fprintf(stderr, "VMP加载模块错误: %s\n", err);
        }
        lua_pushstring(L, err);
        lua_pop(env->L, 1);
        return luaL_error(L, "VMP加载模块错误: %s", err);
    }
    
    copy_value(env->L, L, 0);
    lua_pop(env->L, 1);
    
    return 1;
}

static int lua_vmp_get_global(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    const char *name = luaL_checkstring(L, 2);
    
    lua_getglobal(env->L, name);
    
    copy_value(env->L, L, 0);
    lua_pop(env->L, 1);
    
    return 1;
}

static int lua_vmp_set_debug(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    env->debug_mode = lua_toboolean(L, 2);
    return 0;
}

static int lua_vmp_set_protoshare(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    int enable = lua_toboolean(L, 2);
    
    if (enable && !env->proto_share_enabled) {
        env->proto_share_enabled = 1;
        env->proto_reg = vmp_proto_newregistry(lua_getallocf(env->L, NULL), NULL);
    } else if (!enable && env->proto_share_enabled) {
        env->proto_share_enabled = 0;
        if (env->proto_reg) {
            vmp_proto_freeregistry(env->proto_reg);
            env->proto_reg = NULL;
        }
    }
    
    return 0;
}

static int lua_vmp_get_protoshare(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    lua_pushboolean(L, env->proto_share_enabled);
    return 1;
}

static int lua_vmp_set_owner(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    env->owner = (void *)lua_touserdata(L, 2);
    if (env->owner == NULL) {
        env->owner = env;
    }
    return 0;
}

static int lua_vmp_dofile(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    const char *filename = luaL_checkstring(L, 2);
    
    int status = luaL_dofile(env->L, filename);
    if (status != LUA_OK) {
        const char *err = lua_tostring(env->L, -1);
        if (env->debug_mode) {
            fprintf(stderr, "VMP加载文件错误: %s\n", err);
        }
        lua_pushstring(L, err);
        lua_pop(env->L, 1);
        return luaL_error(L, "VMP加载文件错误: %s", err);
    }
    
    int nresults = lua_gettop(env->L);
    if (nresults > 0) {
        for (int i = 1; i <= nresults; i++) {
            lua_pushvalue(env->L, i);
            copy_value(env->L, L, 0);
        }
        lua_pop(env->L, nresults);
    }
    
    return nresults;
}

static int lua_vmp_dostring(lua_State *L) {
    return lua_vmp_exec(L);
}

static int lua_vmp_close(lua_State *L) {
    return lua_vmp_destroy(L);
}

static int lua_vmp_tostring(lua_State *L) {
    VMP_Env *env = (VMP_Env *)luaL_checkudata(L, 1, "vmp_env");
    lua_pushfstring(L, "VMP: debug=%d, proto_share=%d, owner=%p",
                    env->debug_mode,
                    env->proto_share_enabled,
                    env->owner);
    return 1;
}

/**
 * @brief VMP环境的元方法表
 */
static const luaL_Reg vmp_env_mt[] = {
    {"__gc", lua_vmp_destroy},
    {"__tostring", lua_vmp_tostring},
    {"exec", lua_vmp_exec},
    {"dofile", lua_vmp_dofile},
    {"dostring", lua_vmp_dostring},
    {"set", lua_vmp_set_global},
    {"get", lua_vmp_get_global},
    {"import", lua_vmp_import_module},
    {"require", lua_vmp_require},
    {"sdebug", lua_vmp_set_debug},
    {"sprotoshare", lua_vmp_set_protoshare},
    {"gprotoshare", lua_vmp_get_protoshare},
    {"setowner", lua_vmp_set_owner},
    {"close", lua_vmp_close},
    {NULL, NULL}
};

/**
 * @brief VMP模块的方法表
 */
static const luaL_Reg vmp_module[] = {
    {"create", lua_vmp_create},
    {"描述_元方法_exec_set_get_close_sdebug_sprotoshare_gprotoshare_setowner_close", lua_vmp_create},
    {NULL, NULL}
};


/**
 * @brief VMP模块的初始化函数
 * @param L Lua状态机
 * @return 1
 */
int luaopen_vmp(lua_State *L) {
    luaL_newmetatable(L, "vmp_env");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, vmp_env_mt, 0);
    
    luaL_newlib(L, vmp_module);
    
    return 1;
}

/*
** VMP Proto共享API实现
** 允许多个虚拟机之间共享函数原型
*/

VMP_Env *vmp_create(lua_State *L) {
    VMP_Env *env = (VMP_Env *)lua_newuserdata(L, sizeof(VMP_Env));
    env->L = luaL_newstate();
    if (env->L == NULL) {
        return NULL;
    }
    
    env->debug_mode = 0;
    env->proto_share_enabled = 0;
    env->proto_reg = NULL;
    env->owner = env;
    
    luaL_openlibs(env->L);
    
    luaL_getmetatable(L, "vmp_env");
    lua_setmetatable(L, -2);
    
    return env;
}

void vmp_destroy(VMP_Env *env) {
    if (env == NULL) return;
    if (env->L) {
        if (env->proto_share_enabled && env->proto_reg) {
            vmp_proto_freeregistry(env->proto_reg);
            env->proto_reg = NULL;
        }
        lua_close(env->L);
        env->L = NULL;
    }
}

int vmp_exec(VMP_Env *env, const char *code) {
    if (env == NULL || env->L == NULL || code == NULL) return LUA_ERRRUN;
    return vmp_load_and_exec(env, code);
}

int vmp_dofile(VMP_Env *env, const char *filename) {
    if (env == NULL || env->L == NULL || filename == NULL) return LUA_ERRRUN;
    int status = luaL_dofile(env->L, filename);
    return status;
}

int vmp_set_global(VMP_Env *env, const char *name, int idx) {
    if (env == NULL || env->L == NULL || name == NULL) return 0;
    lua_pushvalue(env->L, idx);
    lua_setglobal(env->L, name);
    return 1;
}

int vmp_get_global(VMP_Env *env, const char *name, int idx) {
    if (env == NULL || env->L == NULL || name == NULL) return 0;
    lua_getglobal(env->L, name);
    return 1;
}

void vmp_set_debug(VMP_Env *env, int debug) {
    if (env == NULL) return;
    env->debug_mode = debug;
}

lua_State *vmp_get_state(VMP_Env *env) {
    if (env == NULL) return NULL;
    return env->L;
}

void vmp_enable_protoshare(VMP_Env *env, int enable) {
    if (env == NULL) return;
    if (enable && !env->proto_share_enabled) {
        env->proto_share_enabled = 1;
        env->proto_reg = vmp_proto_newregistry(lua_getallocf(env->L, NULL), NULL);
    } else if (!enable && env->proto_share_enabled) {
        env->proto_share_enabled = 0;
        if (env->proto_reg) {
            vmp_proto_freeregistry(env->proto_reg);
            env->proto_reg = NULL;
        }
    }
}

int vmp_protoshare_enabled(VMP_Env *env) {
    if (env == NULL) return 0;
    return env->proto_share_enabled;
}

void vmp_register_proto(VMP_Env *env, Proto *proto) {
    if (env == NULL || proto == NULL || !env->proto_share_enabled) return;
    vmp_proto_get(env->proto_reg, proto, env->owner);
}

void vmp_unregister_proto(VMP_Env *env, Proto *proto) {
    if (env == NULL || proto == NULL || !env->proto_share_enabled) return;
    vmp_proto_release(env->proto_reg, proto);
}

int vmp_proto_refcount(Proto *proto) {
    if (proto == NULL) return 0;
    return proto->refcount;
}

void vmp_set_owner(VMP_Env *env) {
    if (env == NULL) return;
    env->owner = env;
}
