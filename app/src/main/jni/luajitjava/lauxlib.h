/*
** $Id: lauxlib.h,v 1.88.1.1 2007/12/27 13:02:25 roberto Exp $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#include <stddef.h>
#include <stdio.h>

#include "lua.h"


/* extra error code for `luaL_load' */
#define LUA_ERRFILE     (LUA_ERRERR+1)

typedef struct luaL_Reg {
  const char *name;
  luajitcore_CFunction func;
} luaL_Reg;

LUALIB_API void (luaL_openlib) (luajitcore_State *L, const char *libname,
                                const luaL_Reg *l, int nup);
LUALIB_API void (luaL_register) (luajitcore_State *L, const char *libname,
                                const luaL_Reg *l);
LUALIB_API int (luaL_getmetafield) (luajitcore_State *L, int obj, const char *e);
LUALIB_API int (luaL_callmeta) (luajitcore_State *L, int obj, const char *e);
LUALIB_API int (luaL_typerror) (luajitcore_State *L, int narg, const char *tname);
LUALIB_API int (luaL_argerror) (luajitcore_State *L, int numarg, const char *extramsg);
LUALIB_API const char *(luaL_checklstring) (luajitcore_State *L, int numArg,
                                                          size_t *l);
LUALIB_API const char *(luaL_optlstring) (luajitcore_State *L, int numArg,
                                          const char *def, size_t *l);
LUALIB_API luajitcore_Number (luaL_checknumber) (luajitcore_State *L, int numArg);
LUALIB_API luajitcore_Number (luaL_optnumber) (luajitcore_State *L, int nArg, luajitcore_Number def);

LUALIB_API luajitcore_Integer (luaL_checkinteger) (luajitcore_State *L, int numArg);
LUALIB_API luajitcore_Integer (luaL_optinteger) (luajitcore_State *L, int nArg,
                                          luajitcore_Integer def);

LUALIB_API void (luaL_checkstack) (luajitcore_State *L, int sz, const char *msg);
LUALIB_API void (luaL_checktype) (luajitcore_State *L, int narg, int t);
LUALIB_API void (luaL_checkany) (luajitcore_State *L, int narg);

LUALIB_API int   (luaL_newmetatable) (luajitcore_State *L, const char *tname);
LUALIB_API void *(luaL_checkudata) (luajitcore_State *L, int ud, const char *tname);

LUALIB_API void (luaL_where) (luajitcore_State *L, int lvl);
LUALIB_API int (luaL_error) (luajitcore_State *L, const char *fmt, ...);

LUALIB_API int (luaL_checkoption) (luajitcore_State *L, int narg, const char *def,
                                   const char *const lst[]);

/* pre-defined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API int (luaL_ref) (luajitcore_State *L, int t);
LUALIB_API void (luaL_unref) (luajitcore_State *L, int t, int ref);

LUALIB_API int (luaL_getn) (luajitcore_State *L, int t);
LUALIB_API void (luaL_setn) (luajitcore_State *L, int t, int n);

LUALIB_API int (luaL_loadfile) (luajitcore_State *L, const char *filename);
LUALIB_API int (luaL_loadbuffer) (luajitcore_State *L, const char *buff, size_t sz,
                                  const char *name);
LUALIB_API int (luaL_loadstring) (luajitcore_State *L, const char *s);

LUALIB_API luajitcore_State *(luaL_newstate) (void);


LUALIB_API const char *(luaL_gsub) (luajitcore_State *L, const char *s, const char *p,
                                                  const char *r);

LUALIB_API const char *(luaL_findtable) (luajitcore_State *L, int idx,
                                         const char *fname, int szhint);

/* From Lua 5.2. */
LUALIB_API int luaL_fileresult(luajitcore_State *L, int stat, const char *fname);
LUALIB_API int luaL_execresult(luajitcore_State *L, int stat);
LUALIB_API int (luaL_loadfilex) (luajitcore_State *L, const char *filename,
				 const char *mode);
LUALIB_API int (luaL_loadbufferx) (luajitcore_State *L, const char *buff, size_t sz,
				   const char *name, const char *mode);
LUALIB_API void luaL_traceback (luajitcore_State *L, luajitcore_State *L1, const char *msg,
				int level);
LUALIB_API void (luaL_setfuncs) (luajitcore_State *L, const luaL_Reg *l, int nup);
LUALIB_API void (luaL_pushmodule) (luajitcore_State *L, const char *modname,
				   int sizehint);
LUALIB_API void *(luaL_testudata) (luajitcore_State *L, int ud, const char *tname);
LUALIB_API void (luaL_setmetatable) (luajitcore_State *L, const char *tname);


/*
** ===============================================================
** some useful macros
** ===============================================================
*/

#define luaL_argcheck(L, cond,numarg,extramsg)	\
		((void)((cond) || luaL_argerror(L, (numarg), (extramsg))))
#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), NULL))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), NULL))
#define luaL_checkint(L,n)	((int)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((int)luaL_optinteger(L, (n), (d)))
#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#define luaL_typename(L,i)	luajitcore_typename(L, luajitcore_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || luajitcore_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || luajitcore_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(luajitcore_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(luajitcore_isnoneornil(L,(n)) ? (d) : f(L,(n)))

/* From Lua 5.2. */
#define luaL_newlibtable(L, l) \
	luajitcore_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)
#define luaL_newlib(L, l)	(luaL_newlibtable(L, l), luaL_setfuncs(L, l, 0))

/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/



typedef struct luaL_Buffer {
  char *p;			/* current position in buffer */
  int lvl;  /* number of strings in the stack (level) */
  luajitcore_State *L;
  char buffer[LUAL_BUFFERSIZE];
} luaL_Buffer;

#define luaL_addchar(B,c) \
  ((void)((B)->p < ((B)->buffer+LUAL_BUFFERSIZE) || luaL_prepbuffer(B)), \
   (*(B)->p++ = (char)(c)))

/* compatibility only */
#define luaL_putchar(B,c)	luaL_addchar(B,c)

#define luaL_addsize(B,n)	((B)->p += (n))

LUALIB_API void (luaL_buffinit) (luajitcore_State *L, luaL_Buffer *B);
LUALIB_API char *(luaL_prepbuffer) (luaL_Buffer *B);
LUALIB_API void (luaL_addlstring) (luaL_Buffer *B, const char *s, size_t l);
LUALIB_API void (luaL_addstring) (luaL_Buffer *B, const char *s);
LUALIB_API void (luaL_addvalue) (luaL_Buffer *B);
LUALIB_API void (luaL_pushresult) (luaL_Buffer *B);


/* }====================================================== */

#endif
