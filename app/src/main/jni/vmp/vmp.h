/*
** vmp.h
** VMP虚拟机公共头文件
** 允许多个虚拟机之间共享函数原型
*/

#ifndef vmp_h
#define vmp_h

#include "lua.h"
#include "lobject.h"

#define VMP_VERSION "1.0.0"

typedef struct VMP_Env VMP_Env;
typedef struct VMPProtoRegistry VMPProtoRegistry;

LUAI_FUNC VMP_Env *vmp_create(lua_State *L);
LUAI_FUNC void vmp_destroy(VMP_Env *env);
LUAI_FUNC int vmp_exec(VMP_Env *env, const char *code);
LUAI_FUNC int vmp_dofile(VMP_Env *env, const char *filename);
LUAI_FUNC int vmp_set_global(VMP_Env *env, const char *name, int idx);
LUAI_FUNC int vmp_get_global(VMP_Env *env, const char *name, int idx);
LUAI_FUNC void vmp_set_debug(VMP_Env *env, int debug);
LUAI_FUNC lua_State *vmp_get_state(VMP_Env *env);
LUAI_FUNC void vmp_enable_protoshare(VMP_Env *env, int enable);
LUAI_FUNC int vmp_protoshare_enabled(VMP_Env *env);
LUAI_FUNC void vmp_register_proto(VMP_Env *env, Proto *proto);
LUAI_FUNC void vmp_unregister_proto(VMP_Env *env, Proto *proto);
LUAI_FUNC int vmp_proto_isshared(Proto *proto);
LUAI_FUNC int vmp_proto_refcount(Proto *proto);
LUAI_FUNC void vmp_set_owner(VMP_Env *env);

#endif
