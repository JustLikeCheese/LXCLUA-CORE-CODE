/*
** Standard library header.
** Copyright (C) 2005-2026 Mike Pall. See Copyright Notice in luajit.h
*/

#ifndef _LUALIB_H
#define _LUALIB_H

#include "lua.h"

#define LUA_FILEHANDLE	"FILE*"

#define LUA_COLIBNAME	"coroutine"
#define LUA_MATHLIBNAME	"math"
#define LUA_STRLIBNAME	"string"
#define LUA_TABLIBNAME	"table"
#define LUA_IOLIBNAME	"io"
#define LUA_OSLIBNAME	"os"
#define LUA_LOADLIBNAME	"package"
#define LUA_DBLIBNAME	"debug"
#define LUA_BITLIBNAME	"bit"
#define LUA_JITLIBNAME	"jit"
#define LUA_FFILIBNAME	"ffi"

LUALIB_API int luajitcoreopen_base(luajitcore_State *L);
LUALIB_API int luajitcoreopen_math(luajitcore_State *L);
LUALIB_API int luajitcoreopen_string(luajitcore_State *L);
LUALIB_API int luajitcoreopen_table(luajitcore_State *L);
LUALIB_API int luajitcoreopen_io(luajitcore_State *L);
LUALIB_API int luajitcoreopen_os(luajitcore_State *L);
LUALIB_API int luajitcoreopen_package(luajitcore_State *L);
LUALIB_API int luajitcoreopen_debug(luajitcore_State *L);
LUALIB_API int luajitcoreopen_bit(luajitcore_State *L);
LUALIB_API int luajitcoreopen_jit(luajitcore_State *L);
LUALIB_API int luajitcoreopen_ffi(luajitcore_State *L);
LUALIB_API int luajitcoreopen_string_buffer(luajitcore_State *L);

LUALIB_API void luaL_openlibs(luajitcore_State *L);

#ifndef luajitcore_assert
#define luajitcore_assert(x)	((void)0)
#endif

#endif
