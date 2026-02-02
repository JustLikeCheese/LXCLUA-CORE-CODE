/*
** vmp_proto.c
** VMP Proto共享管理实现
** 允许多个虚拟机之间共享函数原型以减少内存占用
*/

#define vmp_proto_c
#define LUA_CORE

#include "lprefix.h"
#include <string.h>
#include "lua.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "vmp_proto.h"

static uint32_t vmp_proto_hash_code(const Instruction *code, int size) {
    uint32_t h = 2166136261u;
    for (int i = 0; i < size; i++) {
        h ^= code[i];
        h *= 16777619u;
    }
    return h;
}

static uint32_t vmp_proto_calc_key_hash(const VMPProtoKey *key) {
    uint32_t h = 2166136261u;
    h ^= (uintptr_t)key->source;
    h *= 16777619u;
    h ^= (uint32_t)key->linedefined;
    h *= 16777619u;
    h ^= (uint32_t)key->lastlinedefined;
    h *= 16777619u;
    h ^= (uint32_t)key->numparams;
    h *= 16777619u;
    h ^= (uint32_t)key->is_vararg;
    h *= 16777619u;
    h ^= (uint32_t)key->maxstacksize;
    h *= 16777619u;
    h ^= (uint32_t)key->code_size;
    h *= 16777619u;
    h ^= key->code_hash;
    h *= 16777619u;
    return h;
}

static int vmp_proto_key_equal(const VMPProtoKey *a, const VMPProtoKey *b) {
    if (a->linedefined != b->linedefined) return 0;
    if (a->lastlinedefined != b->lastlinedefined) return 0;
    if (a->numparams != b->numparams) return 0;
    if (a->is_vararg != b->is_vararg) return 0;
    if (a->maxstacksize != b->maxstacksize) return 0;
    if (a->code_size != b->code_size) return 0;
    if (a->code_hash != b->code_hash) return 0;
    if (a->source != b->source && (a->source == NULL || b->source == NULL || strcmp(a->source, b->source) != 0)) {
        return 0;
    }
    return 1;
}

VMPProtoRegistry *vmp_proto_newregistry(lua_Alloc alloc, void *ud) {
    VMPProtoRegistry *reg = (VMPProtoRegistry *)alloc(ud, NULL, 0, sizeof(VMPProtoRegistry));
    if (reg == NULL) return NULL;
    memset(reg, 0, sizeof(*reg));
    reg->alloc = alloc;
    reg->ud = ud;
    return reg;
}

void vmp_proto_freeregistry(VMPProtoRegistry *reg) {
    if (reg == NULL) return;
    for (int i = 0; i < VMP_PROTO_HASH_SIZE; i++) {
        VMPProtoEntry *entry = reg->buckets[i];
        while (entry != NULL) {
            VMPProtoEntry *next = entry->next;
            reg->alloc(reg->ud, entry, sizeof(VMPProtoEntry), 0);
            entry = next;
        }
    }
    reg->alloc(reg->ud, reg, sizeof(VMPProtoRegistry), 0);
}

Proto *vmp_proto_get(VMPProtoRegistry *reg, Proto *proto, void *owner) {
    if (reg == NULL || proto == NULL) return NULL;
    VMPProtoKey key;
    key.source = proto->source ? getstr(proto->source) : NULL;
    key.linedefined = proto->linedefined;
    key.lastlinedefined = proto->lastlinedefined;
    key.numparams = proto->numparams;
    key.is_vararg = proto->is_vararg;
    key.maxstacksize = proto->maxstacksize;
    key.code_size = (size_t)proto->sizecode * sizeof(Instruction);
    key.code_hash = vmp_proto_hash_code(proto->code, proto->sizecode);
    uint32_t bucket = vmp_proto_calc_key_hash(&key) % VMP_PROTO_HASH_SIZE;
    VMPProtoEntry *entry = reg->buckets[bucket];
    while (entry != NULL) {
        if (vmp_proto_key_equal(&entry->key, &key)) {
            entry->refcount++;
            proto->refcount = entry->refcount;
            proto->shared_owner = owner;
            proto->flag |= PF_SHARED;
            return entry->proto;
        }
        entry = entry->next;
    }
    entry = (VMPProtoEntry *)reg->alloc(reg->ud, NULL, sizeof(VMPProtoEntry), 0);
    if (entry == NULL) return NULL;
    entry->key = key;
    entry->proto = proto;
    entry->refcount = 1;
    entry->next = reg->buckets[bucket];
    reg->buckets[bucket] = entry;
    proto->refcount = 1;
    proto->shared_owner = owner;
    proto->flag |= PF_SHARED;
    return proto;
}

void vmp_proto_release(VMPProtoRegistry *reg, Proto *proto) {
    if (reg == NULL || proto == NULL || !(proto->flag & PF_SHARED)) return;
    VMPProtoKey key;
    key.source = proto->source ? getstr(proto->source) : NULL;
    key.linedefined = proto->linedefined;
    key.lastlinedefined = proto->lastlinedefined;
    key.numparams = proto->numparams;
    key.is_vararg = proto->is_vararg;
    key.maxstacksize = proto->maxstacksize;
    key.code_size = (size_t)proto->sizecode * sizeof(Instruction);
    key.code_hash = vmp_proto_hash_code(proto->code, proto->sizecode);
    uint32_t bucket = vmp_proto_calc_key_hash(&key) % VMP_PROTO_HASH_SIZE;
    VMPProtoEntry **pprev = &reg->buckets[bucket];
    VMPProtoEntry *entry = reg->buckets[bucket];
    while (entry != NULL) {
        if (vmp_proto_key_equal(&entry->key, &key) && entry->proto == proto) {
            entry->refcount--;
            proto->refcount = entry->refcount;
            if (entry->refcount <= 0) {
                *pprev = entry->next;
                reg->alloc(reg->ud, entry, sizeof(VMPProtoEntry), 0);
                proto->flag &= ~PF_SHARED;
                proto->refcount = 0;
                proto->shared_owner = NULL;
            }
            return;
        }
        pprev = &entry->next;
        entry = entry->next;
    }
}

int vmp_proto_incref(Proto *proto) {
    if (proto == NULL) return 0;
    proto->refcount++;
    return proto->refcount;
}

int vmp_proto_decref(Proto *proto) {
    if (proto == NULL) return 0;
    proto->refcount--;
    if (proto->refcount < 0) proto->refcount = 0;
    return proto->refcount;
}

int vmp_proto_isshared(Proto *proto) {
    if (proto == NULL) return 0;
    return (proto->flag & PF_SHARED) ? 1 : 0;
}

void vmp_proto_setshared(Proto *proto, int shared) {
    if (proto == NULL) return;
    if (shared) {
        proto->flag |= PF_SHARED;
    } else {
        proto->flag &= ~PF_SHARED;
    }
}
