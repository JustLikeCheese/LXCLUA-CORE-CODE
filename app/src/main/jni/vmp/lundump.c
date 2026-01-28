/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#include "lprefix.h"


#include <limits.h>
#include <string.h>
#include <time.h>
#include <zlib.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstring.h"
#include "ltable.h"
#include "lundump.h"
#include "lzio.h"


#if !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


typedef struct {
  lua_State *L;
  ZIO *Z;
  const char *name;
  time_t timestamp;  /* 动态密钥：时间戳 */
  Table *h;  /* 字符串复用列表：记录已加载的字符串 */
  lua_Unsigned nstr;  /* 已加载字符串计数器 */
} LoadState;


static l_noret error (LoadState *S, const char *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,(n)*sizeof((b)[0]))

static void loadBlock (LoadState *S, void *b, size_t size) {
  if (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
}


#define loadVar(S,x)		loadVector(S,&x,1)


static lu_byte loadByte (LoadState *S) {
  int b = zgetc(S->Z);
  if (b == EOZ)
    error(S, "truncated chunk");
  return cast_byte(b);
}


static size_t loadUnsigned (LoadState *S, size_t limit) {
  size_t x = 0;
  int b;
  limit >>= 7;
  do {
    b = loadByte(S);
    if (x >= limit)
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } while ((b & 0x80) == 0);
  return x;
}


static size_t loadSize (LoadState *S) {
  return loadUnsigned(S, MAX_SIZET);
}


static int loadInt (LoadState *S) {
  return cast_int(loadUnsigned(S, INT_MAX));
}


static lua_Number loadNumber (LoadState *S) {
  lua_Number x;
  loadVar(S, x);
  return x;
}


static lua_Integer loadInteger (LoadState *S) {
  lua_Integer x;
  loadVar(S, x);
  return x;
}


/*
** Load a nullable string into prototype 'p'.
*/
/**
 * 反序列化字符串。如果遇到复用标记，则从列表中获取已保存的字符串；
 * 否则加载新字符串并添加到复用列表中。
 * @param S 反序列化状态
 * @param p 原型（用于GC屏障）
 * @return 反序列化得到的字符串
 */
static TString *loadStringN (LoadState *S, Proto *p) {
  lua_State *L = S->L;
  TString *ts;
  size_t size = loadSize(S);
  
  if (size == 0) {  /* 复用之前保存的字符串？ */
    lua_Unsigned idx = loadSize(S);  /* 获取索引 */
    if (idx == 0) {  /* NULL 字符串 */
      return NULL;
    }
    /* 从复用列表中获取字符串 */
    const TValue *stv = luaH_getint(S->h, cast_int(idx));  /* 通过索引获取 */
    if (ttisnil(stv))
      error(S, "invalid string index");
    ts = tsvalue(stv);
    luaC_objbarrier(L, p, ts);
    return ts;
  }
  
  /* 新字符串：需要解密并创建 */
  if ((size -= 1) <= LUAI_MAXSHORTLEN) {  /* 短字符串 */
    char buff[LUAI_MAXSHORTLEN];
    loadVector(S, buff, size);  /* 加载加密后的字符串 */
    
    /* 对字符串进行XOR解密 */
    for (size_t i = 0; i < size; i++) {
      buff[i] = buff[i] ^ ((char *)&S->timestamp)[i % sizeof(S->timestamp)];
    }
    
    ts = luaS_newlstr(L, buff, size);  /* 创建字符串 */
  }
  else {  /* 长字符串 */
    ts = luaS_createlngstrobj(L, size);
    setsvalue2s(L, L->top.p, ts);  /* 锚定字符串防止被GC回收 */
    luaD_inctop(L);
    char *str_content = getlngstr(ts);
    loadVector(S, str_content, size);  /* 加载加密后的字符串 */
    
    /* 对长字符串进行XOR解密 */
    for (size_t i = 0; i < size; i++) {
      str_content[i] = str_content[i] ^ ((char *)&S->timestamp)[i % sizeof(S->timestamp)];
    }
    
    L->top.p--;  /* 弹出栈顶 */
  }
  
  /* 添加到复用列表 */
  S->nstr++;
  TValue sv;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, cast_int(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
  
  return ts;
}


/*
** Load a non-nullable string into prototype 'p'.
*/
static TString *loadString (LoadState *S, Proto *p) {
  TString *st = loadStringN(S, p);
  if (st == NULL)
    error(S, "bad format for constant string");
  return st;
}


static void loadCode (LoadState *S, Proto *f) {
  int orig_size = loadInt(S);
  size_t compressed_size = loadSize(S);
  char *compressed_data;
  char *decrypted_data;
  uLongf dest_len;
  uLong src_len = compressed_size;
  int i;

  // Read timestamp (password) and store it in LoadState
  loadVar(S, S->timestamp);
  
  // Allocate memory for compressed and decrypted data
  compressed_data = (char *)luaM_malloc_(S->L, compressed_size * sizeof(char), 0);
  decrypted_data = (char *)luaM_malloc_(S->L, compressed_size * sizeof(char), 0);
  
  // Read encrypted data
  loadBlock(S, compressed_data, compressed_size);
  
  // Decrypt data using XOR with timestamp as password
  for (i = 0; i < compressed_size; i++) {
    decrypted_data[i] = compressed_data[i] ^ ((char *)&S->timestamp)[i % sizeof(S->timestamp)];
  }
  
  // Allocate memory for original code
  f->code = luaM_newvectorchecked(S->L, orig_size, Instruction);
  f->sizecode = orig_size;
  
  // Calculate destination length
  dest_len = orig_size * sizeof(Instruction);
  
  // Decompress the code
  if (uncompress((Bytef *)f->code, &dest_len, (const Bytef *)decrypted_data, src_len) != Z_OK) {
    error(S, "decompression failed");
    return;
  }
  
  // Free allocated memory
  luaM_free(S->L, compressed_data);
  luaM_free(S->L, decrypted_data);
}


static void loadFunction(LoadState *S, Proto *f, TString *psource);


static void loadConstants (LoadState *S, Proto *f) {
  int i;
  int n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  for (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  for (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    int t = loadByte(S);
    switch (t) {
      case LUA_VNIL:
        setnilvalue(o);
        break;
      case LUA_VFALSE:
        setbfvalue(o);
        break;
      case LUA_VTRUE:
        setbtvalue(o);
        break;
      case LUA_VNUMFLT:
        setfltvalue(o, loadNumber(S));
        break;
      case LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        break;
      case LUA_VSHRSTR:
      case LUA_VLNGSTR:
        setsvalue2n(S->L, o, loadString(S, f));
        break;
      default: lua_assert(0);
    }
  }
}


static void loadProtos (LoadState *S, Proto *f) {
  int i;
  int n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  for (i = 0; i < n; i++)
    f->p[i] = NULL;
  for (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    loadFunction(S, f->p[i], f->source);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
static void loadUpvalues (LoadState *S, Proto *f) {
  int i, n;
  n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  for (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = NULL;
  for (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


static void loadDebug (LoadState *S, Proto *f) {
  int i, n;
  n = loadInt(S);
  f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
  f->sizelineinfo = n;
  loadVector(S, f->lineinfo, n);
  n = loadInt(S);
  f->abslineinfo = luaM_newvectorchecked(S->L, n, AbsLineInfo);
  f->sizeabslineinfo = n;
  for (i = 0; i < n; i++) {
    f->abslineinfo[i].pc = loadInt(S);
    f->abslineinfo[i].line = loadInt(S);
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  for (i = 0; i < n; i++)
    f->locvars[i].varname = NULL;
  for (i = 0; i < n; i++) {
    f->locvars[i].varname = loadStringN(S, f);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  if (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  for (i = 0; i < n; i++)
    f->upvalues[i].name = loadStringN(S, f);
}


static void loadFunction (LoadState *S, Proto *f, TString *psource) {
  f->source = loadStringN(S, f);
  if (f->source == NULL)  /* no source in dump? */
    f->source = psource;  /* reuse parent's source */
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  f->is_vararg = loadByte(S);
  f->maxstacksize = loadByte(S);
  f->difierline_mode = loadByte(S);  /* 新增：读取自定义标志 */
  f->difierline_magicnum = loadInt(S);  /* 新增：读取自定义版本号 */
  loadVar(S, f->difierline_data);  /* 新增：读取自定义数据字段 */
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadDebug(S, f);
}


static void checkliteral (LoadState *S, const char *s, const char *msg) {
  char buff[sizeof(LUA_SIGNATURE) + sizeof(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  if (memcmp(s, buff, len) != 0)
    error(S, msg);
}


static void fchecksize (LoadState *S, size_t size, const char *tname) {
  if (loadByte(S) != size)
    error(S, luaO_pushfstring(S->L, "%s size mismatch", tname));
}


#define checksize(S,t)	fchecksize(S,sizeof(t),#t)

static void checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  
  // 跳过版本号检查，允许随机版本号
  loadByte(S);
  
  if (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  
  // 解密并检查LUAC_DATA
  const char *original_data = LUAC_DATA;
  size_t data_len = sizeof(LUAC_DATA) - 1;
  char *encrypted_data = (char *)luaM_malloc_(S->L, data_len, 0);
  char *decrypted_data = (char *)luaM_malloc_(S->L, data_len, 0);
  
  // 读取加密的LUAC_DATA
  loadVector(S, encrypted_data, data_len);
  
  // 使用时间戳的翻转作为密钥解密
  time_t reversed_timestamp = 0;
  time_t temp = S->timestamp;
  for (size_t i = 0; i < sizeof(time_t); i++) {
    reversed_timestamp = (reversed_timestamp << 8) | (temp & 0xFF);
    temp >>= 8;
  }
  
  // 解密数据
  for (size_t i = 0; i < data_len; i++) {
    decrypted_data[i] = encrypted_data[i] ^ ((char *)&reversed_timestamp)[i % sizeof(reversed_timestamp)];
  }
  
  // 检查解密后的数据是否与原始LUAC_DATA匹配
  if (memcmp(decrypted_data, original_data, data_len) != 0)
    error(S, "corrupted chunk");
  
  // 释放内存
  luaM_free_(S->L, encrypted_data, data_len);
  luaM_free_(S->L, decrypted_data, data_len);
  
  checksize(S, Instruction);
  checksize(S, lua_Integer);
  checksize(S, lua_Number);
  if (loadInteger(S) != LUAC_INT)
    error(S, "integer format mismatch");
  if (loadNumber(S) != LUAC_NUM)
    error(S, "float format mismatch");
}


/*
** Load precompiled chunk.
*/
/**
 * 反序列化预编译块
 * @param L Lua状态机
 * @param Z 输入流
 * @param name 块名称
 * @return 反序列化得到的闭包
 */
LClosure *luaU_undump(lua_State *L, ZIO *Z, const char *name) {
  LoadState S;
  LClosure *cl;
  if (*name == '@' || *name == '=')
    S.name = name + 1;
  else if (*name == LUA_SIGNATURE[0])
    S.name = "binary string";
  else
    S.name = name;
  S.L = L;
  S.Z = Z;
  S.h = luaH_new(L);  /* 创建字符串复用列表 */
  sethvalue2s(L, L->top.p, S.h);  /* 锚定表防止被GC回收 */
  luaD_inctop(L);
  S.nstr = 0;
  checkHeader(&S);
  cl = luaF_newLclosure(L, loadByte(&S));
  setclLvalue2s(L, L->top.p, cl);
  luaD_inctop(L);
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  loadFunction(&S, cl->p, NULL);
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luai_verifycode(L, cl->p);
  L->top.p--;  /* 弹出字符串复用列表 */
  return cl;
}

