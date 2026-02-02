/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define ldump_c
#define LUA_CORE

#include "lprefix.h"


#include <limits.h>
#include <stddef.h>
#include <time.h>
#include <zlib.h>

#include "lua.h"

#include "lobject.h"
#include "lstate.h"
#include "ltable.h"
#include "lundump.h"


typedef struct {
  lua_State *L;
  lua_Writer writer;
  void *data;
  int strip;
  int status;
  time_t timestamp;
  Table *h;  /* 字符串复用表：记录已保存的字符串 */
  lua_Unsigned nstr;  /* 已保存字符串计数器 */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#define dumpVector(D,v,n)	dumpBlock(D,v,(n)*sizeof((v)[0]))

#define dumpLiteral(D, s)	dumpBlock(D,s,sizeof(s) - sizeof(char))


static void dumpBlock (DumpState *D, const void *b, size_t size) {
  if (D->status == 0 && size > 0) {
    lua_unlock(D->L);
    D->status = (*D->writer)(D->L, b, size, D->data);
    lua_lock(D->L);
  }
}


#define dumpVar(D,x)		dumpVector(D,&x,1)


static void dumpByte (DumpState *D, int y) {
  lu_byte x = (lu_byte)y;
  dumpVar(D, x);
}


/*
** 'dumpSize' buffer size: each byte can store up to 7 bits. (The "+6"
** rounds up the division.)
*/
#define DIBS    ((sizeof(size_t) * CHAR_BIT + 6) / 7)

static void dumpSize (DumpState *D, size_t x) {
  lu_byte buff[DIBS];
  int n = 0;
  do {
    buff[DIBS - (++n)] = x & 0x7f;  /* fill buffer in reverse order */
    x >>= 7;
  } while (x != 0);
  buff[DIBS - 1] |= 0x80;  /* mark last byte */
  dumpVector(D, buff + DIBS - n, n);
}


static void dumpInt (DumpState *D, int x) {
  dumpSize(D, x);
}


static void dumpNumber (DumpState *D, lua_Number x) {
  dumpVar(D, x);
}


static void dumpInteger (DumpState *D, lua_Integer x) {
  dumpVar(D, x);
}


/**
 * 序列化字符串。如果字符串已在复用表中，则输出复用标记和索引；
 * 否则输出字符串长度和内容，并添加到复用表中。
 * @param D 序列化状态
 * @param ts 要序列化的字符串
 */
static void dumpString (DumpState *D, const TString *ts) {
  if (ts == NULL) {
    dumpSize(D, 0);  /* 复用 NULL */
    dumpSize(D, 0);  /* NULL 的特殊索引 */
  }
  else {
    TString *ts_nonconst = (TString *)ts;  /* luaH_getstr 需要非 const 指针 */
    const TValue *o = luaH_getstr(D->h, ts_nonconst);  /* 查找是否已存在 */
    if (!ttisnil(o)) {  /* 字符串已保存过？ */
      dumpSize(D, 0);  /* 复用已保存的字符串 */
      dumpSize(D, cast(lua_Unsigned, ivalue(o)));  /* 输出索引 */
    }
    else {  /* 需要写入并保存字符串 */
      TValue key, value;  /* 用于在哈希表中保存字符串 */
      size_t size;
      const char *s = getlstr(ts, size);
      
      /* 对字符串进行XOR加密，使用动态时间戳密钥 */
      char *encrypted_str = (char *)luaM_malloc_(D->L, size, 0);
      for (size_t i = 0; i < size; i++) {
        encrypted_str[i] = s[i] ^ ((char *)&D->timestamp)[i % sizeof(D->timestamp)];
      }
      
      dumpSize(D, size + 1);
      dumpVector(D, encrypted_str, size);
      luaM_free_(D->L, encrypted_str, size);
      
      D->nstr++;  /* 增加已保存字符串计数 */
      
      /* 需要使用非 const 的 TString* 来构造 key */
      TString *key_ts = (TString *)ts;
      setsvalue(D->L, &key, key_ts);  /* 字符串作为键 */
      setivalue(&value, cast_int(D->nstr));  /* 索引作为值 */
      luaH_set(D->L, D->h, &key, &value);  /* 保存到表中: h[ts] = nstr */
    }
  }
}


static void dumpCode (DumpState *D, const Proto *f) {
  int orig_size = f->sizecode;
  size_t compressed_size;
  uLongf dest_len;
  uLong src_len = orig_size * sizeof(Instruction);
  z_stream strm;
  char *compressed_data;
  char *encrypted_data;
  time_t timestamp;
  int i;

  // Generate timestamp as password
  timestamp = time(NULL);
  
  // Calculate compressed buffer size (worst case: original size + 18 bytes)
  dest_len = compressBound(src_len);
  compressed_data = (char *)luaM_malloc_(D->L, dest_len, 0);
  if (compressed_data == NULL) {
    D->status = LUA_ERRMEM;
    return;
  }

  // Compress the code using simple compress function
  if (compress((Bytef *)compressed_data, &dest_len, (const Bytef *)f->code, src_len) != Z_OK) {
    luaM_free_(D->L, compressed_data, dest_len);
    D->status = LUA_ERRMEM;
    return;
  }
  compressed_size = dest_len;

  // Allocate buffer for encrypted data
  encrypted_data = (char *)luaM_malloc_(D->L, compressed_size, 0);
  if (encrypted_data == NULL) {
    luaM_free_(D->L, compressed_data, dest_len);
    D->status = LUA_ERRMEM;
    return;
  }

  // XOR encryption with timestamp as password
  for (i = 0; i < compressed_size; i++) {
    encrypted_data[i] = compressed_data[i] ^ ((char *)&timestamp)[i % sizeof(timestamp)];
  }

  // Write original size, compressed size, timestamp, and encrypted data
  dumpInt(D, orig_size);
  dumpSize(D, compressed_size);
  dumpVar(D, timestamp);
  dumpBlock(D, encrypted_data, compressed_size);

  // Free allocated memory
  luaM_free_(D->L, compressed_data, dest_len);
  luaM_free_(D->L, encrypted_data, compressed_size);
}


static void dumpFunction(DumpState *D, const Proto *f, TString *psource);

static void dumpConstants (DumpState *D, const Proto *f) {
  int i;
  int n = f->sizek;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    const TValue *o = &f->k[i];
    int tt = ttypetag(o);
    dumpByte(D, tt);
    switch (tt) {
      case LUA_VNUMFLT:
        dumpNumber(D, fltvalue(o));
        break;
      case LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        break;
      case LUA_VSHRSTR:
      case LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        break;
      default:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


static void dumpProtos (DumpState *D, const Proto *f) {
  int i;
  int n = f->sizep;
  dumpInt(D, n);
  for (i = 0; i < n; i++)
    dumpFunction(D, f->p[i], f->source);
}


static void dumpUpvalues (DumpState *D, const Proto *f) {
  int i, n = f->sizeupvalues;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].kind);
  }
}


static void dumpDebug (DumpState *D, const Proto *f) {
  int i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  dumpVector(D, f->lineinfo, n);
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    dumpInt(D, f->abslineinfo[i].pc);
    dumpInt(D, f->abslineinfo[i].line);
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  for (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


static void dumpFunction (DumpState *D, const Proto *f, TString *psource) {
  /* 生成动态时间戳密钥 */
  D->timestamp = time(NULL);
  
  if (D->strip || f->source == psource)
    dumpString(D, NULL);  /* no debug info or same source as its parent */
  else
    dumpString(D, f->source);
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->is_vararg);
  dumpByte(D, f->maxstacksize);
  dumpByte(D, f->difierline_mode);  /* 新增：写入自定义标志 */
  dumpInt(D, f->difierline_magicnum);  /* 新增：写入自定义版本号 */
  dumpVar(D, f->difierline_data);  /* 新增：写入自定义数据字段 */
  dumpCode(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpDebug(D, f);
}


static void dumpHeader (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  
  // 使用时间戳生成随机版本号，保持高位与原版本号一致，低位随机
  int random_version = (LUAC_VERSION & 0xF0) | ((unsigned int)time(NULL) % 0x10);
  dumpByte(D, random_version);
  
  dumpByte(D, LUAC_FORMAT);
  
  // 打乱LUAC_DATA，使用动态密钥的翻转
  const char *original_data = LUAC_DATA;
  size_t data_len = sizeof(LUAC_DATA) - 1;
  char *scrambled_data = (char *)luaM_malloc_(D->L, data_len, 0);
  
  // 使用时间戳的翻转作为密钥
  time_t reversed_timestamp = 0;
  time_t temp = D->timestamp;
  for (size_t i = 0; i < sizeof(time_t); i++) {
    reversed_timestamp = (reversed_timestamp << 8) | (temp & 0xFF);
    temp >>= 8;
  }
  
  // 对LUAC_DATA进行XOR加密，使用翻转后的时间戳作为密钥
  for (size_t i = 0; i < data_len; i++) {
    scrambled_data[i] = original_data[i] ^ ((char *)&reversed_timestamp)[i % sizeof(reversed_timestamp)];
  }
  
  dumpBlock(D, scrambled_data, data_len);
  luaM_free_(D->L, scrambled_data, data_len);
  
  dumpByte(D, sizeof(Instruction));
  dumpByte(D, sizeof(lua_Integer));
  dumpByte(D, sizeof(lua_Number));
  dumpInteger(D, LUAC_INT);
  dumpNumber(D, LUAC_NUM);
}


/*
** dump Lua function as precompiled chunk
*/
/**
 * 序列化Lua函数为预编译块
 * @param L Lua状态机
 * @param f 要序列化的函数原型
 * @param w 写入回调函数
 * @param data 写入回调的上下文数据
 * @param strip 是否剥离调试信息
 * @return 序列化状态
 */
int luaU_dump(lua_State *L, const Proto *f, lua_Writer w, void *data,
              int strip) {
  DumpState D;
  D.L = L;
  D.writer = w;
  D.data = data;
  D.strip = strip;
  D.status = 0;
  D.timestamp = time(NULL);
  D.h = luaH_new(L);  /* 创建字符串复用表 */
  sethvalue2s(L, L->top.p, D.h);  /* 锚定表防止被GC回收 */
  L->top.p++;
  D.nstr = 0;
  dumpHeader(&D);
  dumpByte(&D, f->sizeupvalues);
  dumpFunction(&D, f, NULL);
  return D.status;
}

