/*
** $Id: lua.h,v 1.218.1.5 2008/08/06 13:30:12 roberto Exp $
** Lua - An Extensible Extension Language
** Lua.org, PUC-Rio, Brazil (https://www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef luajitcore_h
#define luajitcore_h

#include <stdarg.h>
#include <stddef.h>


#include "luaconf.h"


#define LUA_VERSION	"Lua 5.1"
#define LUA_RELEASE	"Lua 5.1.4"
#define LUA_VERSION_NUM	501
#define LUA_COPYRIGHT	"Copyright (C) 1994-2008 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo & W. Celes"


/* mark for precompiled code (`<esc>Lua') */
#define	LUA_SIGNATURE	"\033Lua"

/* option for multiple returns in `luajitcore_pcall' and `luajitcore_call' */
#define LUA_MULTRET	(-1)


/*
** pseudo-indices
*/
#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)
#define luajitcore_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))


/* thread status */
#define LUA_OK		0
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5


typedef struct luajitcore_State luajitcore_State;

typedef int (*luajitcore_CFunction) (luajitcore_State *L);


/*
** functions that read/write blocks when loading/dumping Lua chunks
*/
typedef const char * (*luajitcore_Reader) (luajitcore_State *L, void *ud, size_t *sz);

typedef int (*luajitcore_Writer) (luajitcore_State *L, const void* p, size_t sz, void* ud);


/*
** prototype for memory-allocation functions
*/
typedef void * (*luajitcore_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);


/*
** basic types
*/
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8



/* minimum Lua stack available to a C function */
#define LUA_MINSTACK	20


/*
** generic extra include file
*/
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif


/* type of numbers in Lua */
typedef LUA_NUMBER luajitcore_Number;


/* type for integer functions */
typedef LUA_INTEGER luajitcore_Integer;



/*
** state manipulation
*/
LUA_API luajitcore_State *(luajitcore_newstate) (luajitcore_Alloc f, void *ud);
LUA_API void       (luajitcore_close) (luajitcore_State *L);
LUA_API luajitcore_State *(luajitcore_newthread) (luajitcore_State *L);

LUA_API luajitcore_CFunction (luajitcore_atpanic) (luajitcore_State *L, luajitcore_CFunction panicf);


/*
** basic stack manipulation
*/
LUA_API int   (luajitcore_gettop) (luajitcore_State *L);
LUA_API void  (luajitcore_settop) (luajitcore_State *L, int idx);
LUA_API void  (luajitcore_pushvalue) (luajitcore_State *L, int idx);
LUA_API void  (luajitcore_remove) (luajitcore_State *L, int idx);
LUA_API void  (luajitcore_insert) (luajitcore_State *L, int idx);
LUA_API void  (luajitcore_replace) (luajitcore_State *L, int idx);
LUA_API int   (luajitcore_checkstack) (luajitcore_State *L, int sz);

LUA_API void  (luajitcore_xmove) (luajitcore_State *from, luajitcore_State *to, int n);


/*
** access functions (stack -> C)
*/

LUA_API int             (luajitcore_isnumber) (luajitcore_State *L, int idx);
LUA_API int             (luajitcore_isstring) (luajitcore_State *L, int idx);
LUA_API int             (luajitcore_iscfunction) (luajitcore_State *L, int idx);
LUA_API int             (luajitcore_isuserdata) (luajitcore_State *L, int idx);
LUA_API int             (luajitcore_type) (luajitcore_State *L, int idx);
LUA_API const char     *(luajitcore_typename) (luajitcore_State *L, int tp);

LUA_API int            (luajitcore_equal) (luajitcore_State *L, int idx1, int idx2);
LUA_API int            (luajitcore_rawequal) (luajitcore_State *L, int idx1, int idx2);
LUA_API int            (luajitcore_lessthan) (luajitcore_State *L, int idx1, int idx2);

LUA_API luajitcore_Number      (luajitcore_tonumber) (luajitcore_State *L, int idx);
LUA_API luajitcore_Integer     (luajitcore_tointeger) (luajitcore_State *L, int idx);
LUA_API int             (luajitcore_toboolean) (luajitcore_State *L, int idx);
LUA_API const char     *(luajitcore_tolstring) (luajitcore_State *L, int idx, size_t *len);
LUA_API size_t          (luajitcore_objlen) (luajitcore_State *L, int idx);
LUA_API luajitcore_CFunction   (luajitcore_tocfunction) (luajitcore_State *L, int idx);
LUA_API void	       *(luajitcore_touserdata) (luajitcore_State *L, int idx);
LUA_API luajitcore_State      *(luajitcore_tothread) (luajitcore_State *L, int idx);
LUA_API const void     *(luajitcore_topointer) (luajitcore_State *L, int idx);


/*
** push functions (C -> stack)
*/
LUA_API void  (luajitcore_pushnil) (luajitcore_State *L);
LUA_API void  (luajitcore_pushnumber) (luajitcore_State *L, luajitcore_Number n);
LUA_API void  (luajitcore_pushinteger) (luajitcore_State *L, luajitcore_Integer n);
LUA_API void  (luajitcore_pushlstring) (luajitcore_State *L, const char *s, size_t l);
LUA_API void  (luajitcore_pushstring) (luajitcore_State *L, const char *s);
LUA_API const char *(luajitcore_pushvfstring) (luajitcore_State *L, const char *fmt,
                                                      va_list argp);
LUA_API const char *(luajitcore_pushfstring) (luajitcore_State *L, const char *fmt, ...);
LUA_API void  (luajitcore_pushcclosure) (luajitcore_State *L, luajitcore_CFunction fn, int n);
LUA_API void  (luajitcore_pushboolean) (luajitcore_State *L, int b);
LUA_API void  (luajitcore_pushlightuserdata) (luajitcore_State *L, void *p);
LUA_API int   (luajitcore_pushthread) (luajitcore_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API void  (luajitcore_gettable) (luajitcore_State *L, int idx);
LUA_API void  (luajitcore_getfield) (luajitcore_State *L, int idx, const char *k);
LUA_API void  (luajitcore_rawget) (luajitcore_State *L, int idx);
LUA_API void  (luajitcore_rawgeti) (luajitcore_State *L, int idx, int n);
LUA_API void  (luajitcore_createtable) (luajitcore_State *L, int narr, int nrec);
LUA_API void *(luajitcore_newuserdata) (luajitcore_State *L, size_t sz);
LUA_API int   (luajitcore_getmetatable) (luajitcore_State *L, int objindex);
LUA_API void  (luajitcore_getfenv) (luajitcore_State *L, int idx);


/*
** set functions (stack -> Lua)
*/
LUA_API void  (luajitcore_settable) (luajitcore_State *L, int idx);
LUA_API void  (luajitcore_setfield) (luajitcore_State *L, int idx, const char *k);
LUA_API void  (luajitcore_rawset) (luajitcore_State *L, int idx);
LUA_API void  (luajitcore_rawseti) (luajitcore_State *L, int idx, int n);
LUA_API int   (luajitcore_setmetatable) (luajitcore_State *L, int objindex);
LUA_API int   (luajitcore_setfenv) (luajitcore_State *L, int idx);


/*
** `load' and `call' functions (load and run Lua code)
*/
LUA_API void  (luajitcore_call) (luajitcore_State *L, int nargs, int nresults);
LUA_API int   (luajitcore_pcall) (luajitcore_State *L, int nargs, int nresults, int errfunc);
LUA_API int   (luajitcore_cpcall) (luajitcore_State *L, luajitcore_CFunction func, void *ud);
LUA_API int   (luajitcore_load) (luajitcore_State *L, luajitcore_Reader reader, void *dt,
                                        const char *chunkname);

LUA_API int (luajitcore_dump) (luajitcore_State *L, luajitcore_Writer writer, void *data);


/*
** coroutine functions
*/
LUA_API int  (luajitcore_yield) (luajitcore_State *L, int nresults);
LUA_API int  (luajitcore_resume) (luajitcore_State *L, int narg);
LUA_API int  (luajitcore_status) (luajitcore_State *L);

/*
** garbage-collection function and options
*/

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
#define LUA_GCSTEP		5
#define LUA_GCSETPAUSE		6
#define LUA_GCSETSTEPMUL	7
#define LUA_GCISRUNNING		9

LUA_API int (luajitcore_gc) (luajitcore_State *L, int what, int data);


/*
** miscellaneous functions
*/

LUA_API int   (luajitcore_error) (luajitcore_State *L);

LUA_API int   (luajitcore_next) (luajitcore_State *L, int idx);

LUA_API void  (luajitcore_concat) (luajitcore_State *L, int n);

LUA_API luajitcore_Alloc (luajitcore_getallocf) (luajitcore_State *L, void **ud);
LUA_API void luajitcore_setallocf (luajitcore_State *L, luajitcore_Alloc f, void *ud);



/*
** ===============================================================
** some useful macros
** ===============================================================
*/

#define luajitcore_pop(L,n)		luajitcore_settop(L, -(n)-1)

#define luajitcore_newtable(L)		luajitcore_createtable(L, 0, 0)

#define luajitcore_register(L,n,f) (luajitcore_pushcfunction(L, (f)), luajitcore_setglobal(L, (n)))

#define luajitcore_pushcfunction(L,f)	luajitcore_pushcclosure(L, (f), 0)

#define luajitcore_strlen(L,i)		luajitcore_objlen(L, (i))

#define luajitcore_isfunction(L,n)	(luajitcore_type(L, (n)) == LUA_TFUNCTION)
#define luajitcore_istable(L,n)	(luajitcore_type(L, (n)) == LUA_TTABLE)
#define luajitcore_islightuserdata(L,n)	(luajitcore_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define luajitcore_isnil(L,n)		(luajitcore_type(L, (n)) == LUA_TNIL)
#define luajitcore_isboolean(L,n)	(luajitcore_type(L, (n)) == LUA_TBOOLEAN)
#define luajitcore_isthread(L,n)	(luajitcore_type(L, (n)) == LUA_TTHREAD)
#define luajitcore_isnone(L,n)		(luajitcore_type(L, (n)) == LUA_TNONE)
#define luajitcore_isnoneornil(L, n)	(luajitcore_type(L, (n)) <= 0)

#define luajitcore_pushliteral(L, s)	\
	luajitcore_pushlstring(L, "" s, (sizeof(s)/sizeof(char))-1)

#define luajitcore_setglobal(L,s)	luajitcore_setfield(L, LUA_GLOBALSINDEX, (s))
#define luajitcore_getglobal(L,s)	luajitcore_getfield(L, LUA_GLOBALSINDEX, (s))

#define luajitcore_tostring(L,i)	luajitcore_tolstring(L, (i), NULL)



/*
** compatibility macros and functions
*/

#define luajitcore_open()	luaL_newstate()

#define luajitcore_getregistry(L)	luajitcore_pushvalue(L, LUA_REGISTRYINDEX)

#define luajitcore_getgccount(L)	luajitcore_gc(L, LUA_GCCOUNT, 0)

#define luajitcore_Chunkreader		luajitcore_Reader
#define luajitcore_Chunkwriter		luajitcore_Writer


/* hack */
LUA_API void luajitcore_setlevel	(luajitcore_State *from, luajitcore_State *to);


/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILRET 4


/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)

typedef struct luajitcore_Debug luajitcore_Debug;  /* activation record */


/* Functions to be called by the debuger in specific events */
typedef void (*luajitcore_Hook) (luajitcore_State *L, luajitcore_Debug *ar);


LUA_API int luajitcore_getstack (luajitcore_State *L, int level, luajitcore_Debug *ar);
LUA_API int luajitcore_getinfo (luajitcore_State *L, const char *what, luajitcore_Debug *ar);
LUA_API const char *luajitcore_getlocal (luajitcore_State *L, const luajitcore_Debug *ar, int n);
LUA_API const char *luajitcore_setlocal (luajitcore_State *L, const luajitcore_Debug *ar, int n);
LUA_API const char *luajitcore_getupvalue (luajitcore_State *L, int funcindex, int n);
LUA_API const char *luajitcore_setupvalue (luajitcore_State *L, int funcindex, int n);
LUA_API int luajitcore_sethook (luajitcore_State *L, luajitcore_Hook func, int mask, int count);
LUA_API luajitcore_Hook luajitcore_gethook (luajitcore_State *L);
LUA_API int luajitcore_gethookmask (luajitcore_State *L);
LUA_API int luajitcore_gethookcount (luajitcore_State *L);

/* From Lua 5.2. */
LUA_API void *luajitcore_upvalueid (luajitcore_State *L, int idx, int n);
LUA_API void luajitcore_upvaluejoin (luajitcore_State *L, int idx1, int n1, int idx2, int n2);
LUA_API int luajitcore_loadx (luajitcore_State *L, luajitcore_Reader reader, void *dt,
		       const char *chunkname, const char *mode);
LUA_API const luajitcore_Number *luajitcore_version (luajitcore_State *L);
LUA_API void luajitcore_copy (luajitcore_State *L, int fromidx, int toidx);
LUA_API luajitcore_Number luajitcore_tonumberx (luajitcore_State *L, int idx, int *isnum);
LUA_API luajitcore_Integer luajitcore_tointegerx (luajitcore_State *L, int idx, int *isnum);

/* From Lua 5.3. */
LUA_API int luajitcore_isyieldable (luajitcore_State *L);


struct luajitcore_Debug {
  int event;
  const char *name;	/* (n) */
  const char *namewhat;	/* (n) `global', `local', `field', `method' */
  const char *what;	/* (S) `Lua', `C', `main', `tail' */
  const char *source;	/* (S) */
  int currentline;	/* (l) */
  int nups;		/* (u) number of upvalues */
  int linedefined;	/* (S) */
  int lastlinedefined;	/* (S) */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  int i_ci;  /* active function */
};

/* }====================================================================== */


/******************************************************************************
* Copyright (C) 1994-2008 Lua.org, PUC-Rio.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/


#endif
