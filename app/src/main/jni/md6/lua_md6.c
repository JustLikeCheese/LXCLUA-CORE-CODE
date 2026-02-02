/**
 * MD6 哈希算法的 Lua C 模块封装
 * 提供 MD6 哈希函数的 Lua 接口
 *
 * @author  DifierLine
 * @date    2026-01-19
 */

#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include "md6.h"

#define MD6_DEFAULT_BITS 256

static const char hex_chars[] = "0123456789abcdef";

static void bin2hex(const unsigned char *bin, size_t bin_len, char *hex_out) {
  size_t i;
  for (i = 0; i < bin_len; i++) {
    hex_out[i * 2] = hex_chars[(bin[i] >> 4) & 0x0F];
    hex_out[i * 2 + 1] = hex_chars[bin[i] & 0x0F];
  }
  hex_out[bin_len * 2] = '\0';
}

typedef struct {
  md6_state state;
  int initialized;
} md6_user_data_t;

static int lmd6_hash(lua_State *L) {
  const unsigned char *message;
  unsigned char hashval[64];
  char hex_out[128];
  size_t len;
  int d = luaL_optinteger(L, 2, MD6_DEFAULT_BITS);
  
  if (d < 1 || d > 512) {
    return luaL_error(L, "digest length must be between 1 and 512 bits");
  }
  
  message = (const unsigned char *)luaL_checklstring(L, 1, &len);
  
  if (md6_hash(d, (unsigned char *)message, len * 8, hashval) != MD6_SUCCESS) {
    return luaL_error(L, "md6_hash failed");
  }
  
  bin2hex(hashval, d / 8, hex_out);
  lua_pushstring(L, hex_out);
  return 1;
}

static int lmd6_new(lua_State *L) {
  md6_user_data_t *ud;
  int d = luaL_optinteger(L, 1, MD6_DEFAULT_BITS);
  
  if (d < 1 || d > 512) {
    return luaL_error(L, "digest length must be between 1 and 512 bits");
  }
  
  ud = (md6_user_data_t *)lua_newuserdata(L, sizeof(md6_user_data_t));
  luaL_getmetatable(L, "md6对象元表");
  lua_setmetatable(L, -2);
  
  if (md6_init(&ud->state, d) != MD6_SUCCESS) {
    return luaL_error(L, "md6_init failed");
  }
  
  ud->initialized = 1;
  return 1;
}

static int lmd6_update(lua_State *L) {
  md6_user_data_t *ud;
  const unsigned char *data;
  size_t len;
  
  ud = (md6_user_data_t *)luaL_checkudata(L, 1, "md6对象元表");
  if (!ud->initialized) {
    return luaL_error(L, "md6 object not initialized");
  }
  
  data = (const unsigned char *)luaL_checklstring(L, 2, &len);
  
  if (md6_update(&ud->state, (unsigned char *)data, len * 8) != MD6_SUCCESS) {
    return luaL_error(L, "md6_update failed");
  }
  
  lua_pushvalue(L, 1);
  return 1;
}

static int lmd6_final(lua_State *L) {
  md6_user_data_t *ud;
  unsigned char hashval[64];
  char hex_out[128];
  int d;
  
  ud = (md6_user_data_t *)luaL_checkudata(L, 1, "md6对象元表");
  if (!ud->initialized) {
    return luaL_error(L, "md6 object not initialized");
  }
  
  d = ud->state.d;
  
  if (md6_final(&ud->state, hashval) != MD6_SUCCESS) {
    return luaL_error(L, "md6_final failed");
  }
  
  bin2hex(hashval, d / 8, hex_out);
  lua_pushstring(L, hex_out);
  return 1;
}

static int lmd6_gc(lua_State *L) {
  md6_user_data_t *ud;
  ud = (md6_user_data_t *)luaL_checkudata(L, 1, "md6对象元表");
  ud->initialized = 0;
  return 0;
}

static int lmd6_tostring(lua_State *L) {
  lua_pushstring(L, "md6 object");
  return 1;
}

static int lmd6_digest(lua_State *L) {
  const unsigned char *message;
  unsigned char hashval[64];
  char hex_out[128];
  size_t len;
  int d;
  
  d = luaL_checkinteger(L, 1);
  if (d < 1 || d > 512) {
    return luaL_error(L, "digest length must be between 1 and 512 bits");
  }
  
  message = (const unsigned char *)luaL_checklstring(L, 2, &len);
  
  if (md6_hash(d, (unsigned char *)message, len * 8, hashval) != MD6_SUCCESS) {
    return luaL_error(L, "md6_hash failed");
  }
  
  bin2hex(hashval, d / 8, hex_out);
  lua_pushstring(L, hex_out);
  return 1;
}

static const struct luaL_Reg md6lib_f[] = {
  {"digest", lmd6_digest},
  {"hash", lmd6_hash},
  {NULL, NULL}
};

static const struct luaL_Reg md6lib_m[] = {
  {"update", lmd6_update},
  {"final", lmd6_final},
  {"__gc", lmd6_gc},
  {"__tostring", lmd6_tostring},
  {NULL, NULL}
};

static void create_metatable(lua_State *L) {
  luaL_newmetatable(L, "md6对象元表");
  luaL_setfuncs(L, md6lib_m, 0);
  lua_pop(L, 1);
}

#if defined(__GNUC__)
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

/**
 * MD6 模块的入口函数，用于注册 MD6 哈希函数到 Lua 状态机
 * @param L Lua 状态机指针
 * @return  返回压入栈中的表数量（始终为 1）
 */
EXPORT int luaopen_md6(lua_State *L) {
  luaL_newlib(L, md6lib_f);
  create_metatable(L);
  
  lua_pushliteral(L, "_COPYRIGHT");
  lua_pushliteral(L, "Copyright (C) 2009 Ronald L. Rivest, DifierLine");
  lua_settable(L, -3);
  
  lua_pushliteral(L, "_DESCRIPTION");
  lua_pushliteral(L, "MD6 cryptographic hash function");
  lua_settable(L, -3);
  
  lua_pushliteral(L, "_VERSION");
  lua_pushliteral(L, "MD6 1.0 - DifierLine 2026-01-19");
  lua_settable(L, -3);
  
  return 1;
}
