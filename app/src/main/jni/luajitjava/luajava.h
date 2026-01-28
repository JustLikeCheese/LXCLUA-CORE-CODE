#ifndef luajava_h
#define luajava_h


#include <jni.h>
#include <stdio.h>
#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "luajit.h"

typedef struct {
    int type;
    int index;
} java_object;

typedef struct {
    luajitcore_State *L;
    int threadRef;
} luaJavaThread;

 JNIEnv *checkEnv(luajitcore_State *L);

 void pushJNIEnv(JNIEnv *env, luajitcore_State *L);

 jlong checkIndex(luajitcore_State *L);

 java_object *checkJavaObject(luajitcore_State *L, int idx);

 jobject *toJavaObject(luajitcore_State *L, int idx);

 void checkError(JNIEnv *javaEnv, luajitcore_State *L);

 JNIEnv *getEnvFromState(luajitcore_State * L);

 int isJavaObject(luajitcore_State * L, int idx);

 int pushJavaObject(luajitcore_State *L, const char *name,int idx,int isclass);

 int pushJavaThread(luajitcore_State *L, luajitcore_State *thread, int threadRef);

 luaJavaThread *checkJavaThread(luajitcore_State *L, int idx);

 int luaJIT_setmode(luajitcore_State *L, int idx, int mode);

 void luaJIT_profile_stop(luajitcore_State *L);

 const char *luaJIT_profile_dumpstack(luajitcore_State *L, const char *fmt, int depth, size_t *len);

#endif
