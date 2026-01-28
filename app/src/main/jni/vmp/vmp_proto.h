/*
** vmp_proto.h
** VMP Proto共享管理头文件
** 允许多个虚拟机之间共享函数原型以减少内存占用
*/

#ifndef vmp_proto_h
#define vmp_proto_h

#include "lua.h"
#include "lobject.h"

#define VMP_PROTO_HASH_SIZE 64

typedef struct VMPProtoKey {
    const char *source;
    int linedefined;
    int lastlinedefined;
    int numparams;
    int is_vararg;
    int maxstacksize;
    size_t code_size;
    uint32_t code_hash;
} VMPProtoKey;

typedef struct VMPProtoEntry {
    VMPProtoKey key;
    struct Proto *proto;
    int refcount;
    struct VMPProtoEntry *next;
} VMPProtoEntry;

typedef struct VMPProtoRegistry {
    VMPProtoEntry *buckets[VMP_PROTO_HASH_SIZE];
    lua_Alloc alloc;
    void *ud;
} VMPProtoRegistry;

LUAI_FUNC VMPProtoRegistry *vmp_proto_newregistry(lua_Alloc alloc, void *ud);
LUAI_FUNC void vmp_proto_freeregistry(VMPProtoRegistry *reg);
LUAI_FUNC struct Proto *vmp_proto_get(VMPProtoRegistry *reg, struct Proto *proto, void *owner);
LUAI_FUNC void vmp_proto_release(VMPProtoRegistry *reg, struct Proto *proto);
LUAI_FUNC int vmp_proto_incref(struct Proto *proto);
LUAI_FUNC int vmp_proto_decref(struct Proto *proto);
LUAI_FUNC int vmp_proto_isshared(struct Proto *proto);
LUAI_FUNC void vmp_proto_setshared(struct Proto *proto, int shared);

#endif
