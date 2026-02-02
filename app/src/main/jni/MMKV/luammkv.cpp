/*
 MMKV Lua 绑定模块
 封装 Tencent MMKV 高性能键值存储库的 Lua 接口
 DifierLine 2026 01/06
 */

#include <android/log.h>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#ifdef __cplusplus
}
#endif

#include "MMKV/MMKV.h"
#include "MMKV/MMKVPredef.h"

#ifndef LOG_TAG
#define LOG_TAG "luammkv"
#endif

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace mmkv;

static std::unordered_map<std::string, MMKV *> mmkvInstances;
static bool mmkvInitialized = false;

static int lua_mmkv_initialize(lua_State *L) {
    if (lua_gettop(L) < 1) {
        lua_pushnil(L);
        lua_pushstring(L, "必须传入目录路径");
        return 2;
    }
    
    const char *rootDir = luaL_checkstring(L, 1);
    
    if (!mmkvInitialized) {
        MMKV::initializeMMKV(rootDir);
        mmkvInitialized = true;
    }
    
    lua_pushboolean(L, true);
    lua_pushstring(L, "成功");
    return 2;
}

static int lua_mmkv_finalize(lua_State *L) {
    MMKV::onExit();
    mmkvInitialized = false;
    return 0;
}

static MMKV *getMMKVFromLua(lua_State *L, int index) {
    const char *name = luaL_checkstring(L, index);
    auto it = mmkvInstances.find(name);
    if (it == mmkvInstances.end()) {
        return nullptr;
    }
    return it->second;
}

static int lua_mmkv_open(lua_State *L) {
    const char *mmapID = luaL_checkstring(L, 1);
    int size = luaL_optinteger(L, 2, 0);
    int mode = luaL_optinteger(L, 3, MMKV_SINGLE_PROCESS);
    const char *cryptKey = luaL_optstring(L, 4, nullptr);
    const char *rootPath = luaL_optstring(L, 5, nullptr);
    size_t expectedCapacity = luaL_optinteger(L, 6, 0);
    bool aes256 = lua_toboolean(L, 7);
    
    auto it = mmkvInstances.find(mmapID);
    if (it != mmkvInstances.end()) {
        lua_pushstring(L, mmapID);
        lua_pushboolean(L, true);
        lua_pushstring(L, "已存在");
        return 3;
    }
    
    MMKV *mmkv = nullptr;
    std::string cryptKeyStr, rootPathStr;
    const std::string *pCryptKey = nullptr;
    const std::string *pRootPath = nullptr;
    
    if (cryptKey && strlen(cryptKey) > 0) {
        cryptKeyStr = cryptKey;
        pCryptKey = &cryptKeyStr;
    }
    
    if (rootPath && strlen(rootPath) > 0) {
        rootPathStr = rootPath;
        pRootPath = &rootPathStr;
    }
    
    mmkv = MMKV::mmkvWithID(mmapID, size, (MMKVMode)mode, pCryptKey, pRootPath, expectedCapacity, aes256);
    
    if (mmkv) {
        mmkvInstances[mmapID] = mmkv;
        lua_pushstring(L, mmapID);
        lua_pushboolean(L, true);
        lua_pushstring(L, "成功");
        return 3;
    }
    
    lua_pushnil(L);
    lua_pushboolean(L, false);
    lua_pushstring(L, "创建MMKV实例失败");
    return 3;
}

static int lua_mmkv_close(lua_State *L) {
    const char *mmapID = luaL_checkstring(L, 1);
    auto it = mmkvInstances.find(mmapID);
    if (it != mmkvInstances.end()) {
        it->second->close();
        mmkvInstances.erase(it);
    }
    return 0;
}

static int lua_mmkv_setBool(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    bool value = lua_toboolean(L, 3);
    bool result = mmkv->set(value, key);
    
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_setInteger(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    
    int valueType = lua_type(L, 3);
    bool result = false;
    
    if (valueType == LUA_TNUMBER) {
        lua_Number value = lua_tonumber(L, 3);
        if (lua_isinteger(L, 3)) {
            if (value >= 0) {
                result = mmkv->set((uint64_t)value, key);
            } else {
                result = mmkv->set((int64_t)value, key);
            }
        } else {
            result = mmkv->set((double)value, key);
        }
    } else {
        return luaL_error(L, "值必须是数字");
    }
    
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_setString(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        lua_pushnil(L);
        lua_pushboolean(L, false);
        lua_pushstring(L, "无效的MMKV实例");
        return 3;
    }
    
    const char *key = luaL_checkstring(L, 2);
    const char *value = luaL_checkstring(L, 3);
    
    bool result = mmkv->set(value, key);
    lua_pushboolean(L, result);
    lua_pushboolean(L, result);
    lua_pushstring(L, result ? "成功" : "设置失败");
    return 3;
}

static int lua_mmkv_setFloat(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    float value = (float)luaL_checknumber(L, 3);
    
    bool result = mmkv->set(value, key);
    
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_setDouble(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    double value = luaL_checknumber(L, 3);
    
    bool result = mmkv->set(value, key);
    
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_getBool(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    bool defaultValue = lua_toboolean(L, 3);
    
    bool value = mmkv->getBool(key, defaultValue);
    
    lua_pushboolean(L, value);
    return 1;
}

static int lua_mmkv_getInteger(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    lua_Number defaultValue = luaL_optnumber(L, 3, 0);
    
    int64_t value = mmkv->getInt64(key, (int64_t)defaultValue);
    
    lua_pushinteger(L, value);
    return 1;
}

static int lua_mmkv_getNumber(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    lua_Number defaultValue = luaL_optnumber(L, 3, 0);
    
    double value = mmkv->getDouble(key, defaultValue);
    
    lua_pushnumber(L, value);
    return 1;
}

static int lua_mmkv_getString(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    const char *defaultValue = luaL_optstring(L, 3, "");
    
    std::string result;
    if (mmkv->getString(key, result)) {
        lua_pushstring(L, result.c_str());
    } else {
        lua_pushstring(L, defaultValue);
    }
    return 1;
}

static int lua_mmkv_getBytes(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    
    mmkv::MMBuffer buffer = mmkv->getBytes(key);
    if (buffer.length() > 0) {
        lua_pushlstring(L, (const char *)buffer.getPtr(), buffer.length());
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int lua_mmkv_putBytes(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    size_t len = 0;
    const char *value = nullptr;
    
    if (lua_type(L, 3) == LUA_TSTRING) {
        value = lua_tolstring(L, 3, &len);
    } else {
        return luaL_error(L, "预期为字符串或用户数据");
    }
    
    mmkv::MMBuffer buffer((void *)value, len);
    bool result = mmkv->set(buffer, key);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_getVector(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    
    std::vector<std::string> result;
    if (mmkv->getVector(key, result)) {
        lua_newtable(L);
        for (size_t i = 0; i < result.size(); i++) {
            lua_pushinteger(L, i + 1);
            lua_pushstring(L, result[i].c_str());
            lua_settable(L, -3);
        }
    } else {
        lua_newtable(L);
    }
    return 1;
}

static int lua_mmkv_setVector(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    
    if (lua_type(L, 3) != LUA_TTABLE) {
        return luaL_error(L, "预期为表格");
    }
    
    std::vector<std::string> values;
    lua_pushnil(L);
    while (lua_next(L, 3) != 0) {
        const char *value = luaL_checkstring(L, -1);
        values.push_back(value);
        lua_pop(L, 1);
    }
    
    bool result = mmkv->set(values, key);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_getValueSize(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    bool actualSize = lua_toboolean(L, 3);
    
    size_t size = mmkv->getValueSize(key, actualSize);
    lua_pushinteger(L, size);
    return 1;
}

static int lua_mmkv_lock(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    mmkv->lock();
    return 0;
}

static int lua_mmkv_unlock(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    mmkv->unlock();
    return 0;
}

static int lua_mmkv_tryLock(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    bool result = mmkv->try_lock();
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_enableExpire(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    uint32_t seconds = (uint32_t)luaL_optinteger(L, 2, 0);
    bool result = mmkv->enableAutoKeyExpire(seconds);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_disableExpire(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    bool result = mmkv->disableAutoKeyExpire();
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_importFrom(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *srcId = luaL_checkstring(L, 2);
    int mode = luaL_optinteger(L, 3, MMKV_SINGLE_PROCESS);
    
    MMKV *src = MMKV::mmkvWithID(srcId, mode);
    if (src) {
        size_t count = mmkv->importFrom(src);
        lua_pushinteger(L, count);
    } else {
        lua_pushinteger(L, 0);
    }
    return 1;
}

static int lua_mmkv_checkContentChanged(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    mmkv->checkContentChanged();
    return 0;
}

static int lua_mmkv_setLogLevel(lua_State *L) {
    int level = luaL_optinteger(L, 1, 0);
    MMKV::setLogLevel((MMKVLogLevel)level);
    return 0;
}

static int lua_mmkv_isExpirationEnabled(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    bool result = mmkv->isExpirationEnabled();
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_isEncryptionEnabled(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    bool result = mmkv->isEncryptionEnabled();
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_containsKey(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    bool result = mmkv->containsKey(key);
    
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_removeValueForKey(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *key = luaL_checkstring(L, 2);
    bool result = mmkv->removeValueForKey(key);
    
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_removeValuesForKeys(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    if (lua_type(L, 2) != LUA_TTABLE) {
        return luaL_error(L, "预期为键的表格");
    }
    
    std::vector<std::string> keys;
    lua_pushnil(L);
    while (lua_next(L, 2) != 0) {
        const char *key = luaL_checkstring(L, -1);
        keys.push_back(key);
        lua_pop(L, 1);
    }
    
    bool result = mmkv->removeValuesForKeys(keys);
    
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_clearAll(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    bool keepSpace = lua_toboolean(L, 2);
    mmkv->clearAll(keepSpace);
    
    return 0;
}

static int lua_mmkv_count(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    bool filterExpire = lua_toboolean(L, 2);
    size_t count = mmkv->count(filterExpire);
    
    lua_pushinteger(L, count);
    return 1;
}

static int lua_mmkv_allKeys(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    bool filterExpire = lua_toboolean(L, 2);
    std::vector<std::string> keys = mmkv->allKeys(filterExpire);
    
    lua_newtable(L);
    for (size_t i = 0; i < keys.size(); i++) {
        lua_pushinteger(L, i + 1);
        lua_pushstring(L, keys[i].c_str());
        lua_settable(L, -3);
    }
    
    return 1;
}

static int lua_mmkv_totalSize(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    size_t totalSize = mmkv->totalSize();
    
    lua_pushinteger(L, totalSize);
    return 1;
}

static int lua_mmkv_actualSize(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    size_t actualSize = mmkv->actualSize();
    
    lua_pushinteger(L, actualSize);
    return 1;
}

static int lua_mmkv_sync(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    bool async = lua_toboolean(L, 2);
    mmkv->sync(async ? MMKV_ASYNC : MMKV_SYNC);
    
    return 0;
}

static int lua_mmkv_trim(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    mmkv->trim();
    
    return 0;
}

static int lua_mmkv_clearMemoryCache(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    bool keepSpace = lua_toboolean(L, 2);
    mmkv->clearMemoryCache(keepSpace);
    
    return 0;
}

static int lua_mmkv_isMultiProcess(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    bool result = mmkv->isMultiProcess();
    
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_isReadOnly(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    bool result = mmkv->isReadOnly();
    
    lua_pushboolean(L, result);
    return 1;
}

#ifndef MMKV_DISABLE_CRYPT
static int lua_mmkv_cryptKey(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    std::string cryptKey = mmkv->cryptKey();
    
    if (cryptKey.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, cryptKey.c_str());
    }
    return 1;
}

static int lua_mmkv_reKey(lua_State *L) {
    MMKV *mmkv = getMMKVFromLua(L, 1);
    if (!mmkv) {
        return luaL_error(L, "无效的MMKV实例");
    }
    
    const char *newCryptKey = luaL_optstring(L, 2, nullptr);
    bool aes256 = lua_toboolean(L, 3);
    
    if (newCryptKey) {
        bool result = mmkv->reKey(newCryptKey, aes256);
        lua_pushboolean(L, result);
    } else {
        mmkv->reKey("", aes256);
        lua_pushboolean(L, true);
    }
    return 1;
}
#endif

static int lua_mmkv_getRootDir(lua_State *L) {
    const MMKVPath_t &rootDir = MMKV::getRootDir();
    
#if defined(MMKV_WIN32)
    std::string dir = MMKVPath_t2String(rootDir);
    lua_pushstring(L, dir.c_str());
#else
    lua_pushstring(L, rootDir.c_str());
#endif
    return 1;
}

static int lua_mmkv_checkExist(lua_State *L) {
    const char *mmapID = luaL_checkstring(L, 1);
    const char *relatePath = luaL_optstring(L, 2, nullptr);
    
    bool result = false;
    if (relatePath && strlen(relatePath) > 0) {
        std::string pathStr = relatePath;
        result = MMKV::checkExist(mmapID, &pathStr);
    } else {
        result = MMKV::checkExist(mmapID);
    }
    
    lua_pushboolean(L, result);
    return 1;
}

static int lua_mmkv_removeStorage(lua_State *L) {
    const char *mmapID = luaL_checkstring(L, 1);
    const char *relatePath = luaL_optstring(L, 2, nullptr);
    
    bool result = false;
    if (relatePath && strlen(relatePath) > 0) {
        std::string pathStr = relatePath;
        result = MMKV::removeStorage(mmapID, &pathStr);
    } else {
        result = MMKV::removeStorage(mmapID);
    }
    
    lua_pushboolean(L, result);
    return 1;
}

static const luaL_Reg mmkv_functions[] = {
    {"init", lua_mmkv_initialize},
    {"exit", lua_mmkv_finalize},
    {"open", lua_mmkv_open},
    {"close", lua_mmkv_close},
    {"put", lua_mmkv_setBool},
    {"putInt", lua_mmkv_setInteger},
    {"putStr", lua_mmkv_setString},
    {"putFloat", lua_mmkv_setFloat},
    {"putDouble", lua_mmkv_setDouble},
    {"putVec", lua_mmkv_setVector},
    {"putBytes", lua_mmkv_putBytes},
    {"get", lua_mmkv_getBool},
    {"getInt", lua_mmkv_getInteger},
    {"getStr", lua_mmkv_getString},
    {"getFloat", lua_mmkv_getNumber},
    {"getDouble", lua_mmkv_getNumber},
    {"getBytes", lua_mmkv_getBytes},
    {"getVec", lua_mmkv_getVector},
    {"valueSize", lua_mmkv_getValueSize},
    {"lock", lua_mmkv_lock},
    {"unlock", lua_mmkv_unlock},
    {"tryLock", lua_mmkv_tryLock},
    {"enableExpire", lua_mmkv_enableExpire},
    {"disableExpire", lua_mmkv_disableExpire},
    {"import", lua_mmkv_importFrom},
    {"checkChanged", lua_mmkv_checkContentChanged},
    {"setLogLevel", lua_mmkv_setLogLevel},
    {"isExpireEnabled", lua_mmkv_isExpirationEnabled},
    {"isCryptEnabled", lua_mmkv_isEncryptionEnabled},
    {"has", lua_mmkv_containsKey},
    {"del", lua_mmkv_removeValueForKey},
    {"dels", lua_mmkv_removeValuesForKeys},
    {"clear", lua_mmkv_clearAll},
    {"size", lua_mmkv_count},
    {"keys", lua_mmkv_allKeys},
    {"total", lua_mmkv_totalSize},
    {"used", lua_mmkv_actualSize},
    {"flush", lua_mmkv_sync},
    {"trim", lua_mmkv_trim},
    {"free", lua_mmkv_clearMemoryCache},
    {"isMulti", lua_mmkv_isMultiProcess},
    {"isRead", lua_mmkv_isReadOnly},
#ifndef MMKV_DISABLE_CRYPT
    {"cryptKey", lua_mmkv_cryptKey},
    {"reKey", lua_mmkv_reKey},
#endif
    {"root", lua_mmkv_getRootDir},
    {"exists", lua_mmkv_checkExist},
    {"remove", lua_mmkv_removeStorage},
    {nullptr, nullptr}
};

static const luaL_Reg mmkv_instance_methods[] = {
    {"put", lua_mmkv_setBool},
    {"putInt", lua_mmkv_setInteger},
    {"putStr", lua_mmkv_setString},
    {"putFloat", lua_mmkv_setFloat},
    {"putDouble", lua_mmkv_setDouble},
    {"putVec", lua_mmkv_setVector},
    {"putBytes", lua_mmkv_putBytes},
    {"get", lua_mmkv_getBool},
    {"getInt", lua_mmkv_getInteger},
    {"getStr", lua_mmkv_getString},
    {"getFloat", lua_mmkv_getNumber},
    {"getDouble", lua_mmkv_getNumber},
    {"getBytes", lua_mmkv_getBytes},
    {"getVec", lua_mmkv_getVector},
    {"valueSize", lua_mmkv_getValueSize},
    {"lock", lua_mmkv_lock},
    {"unlock", lua_mmkv_unlock},
    {"tryLock", lua_mmkv_tryLock},
    {"enableExpire", lua_mmkv_enableExpire},
    {"disableExpire", lua_mmkv_disableExpire},
    {"import", lua_mmkv_importFrom},
    {"checkChanged", lua_mmkv_checkContentChanged},
    {"isExpireEnabled", lua_mmkv_isExpirationEnabled},
    {"isCryptEnabled", lua_mmkv_isEncryptionEnabled},
    {"has", lua_mmkv_containsKey},
    {"del", lua_mmkv_removeValueForKey},
    {"dels", lua_mmkv_removeValuesForKeys},
    {"clear", lua_mmkv_clearAll},
    {"size", lua_mmkv_count},
    {"keys", lua_mmkv_allKeys},
    {"total", lua_mmkv_totalSize},
    {"used", lua_mmkv_actualSize},
    {"flush", lua_mmkv_sync},
    {"trim", lua_mmkv_trim},
    {"free", lua_mmkv_clearMemoryCache},
    {"isMulti", lua_mmkv_isMultiProcess},
    {"isRead", lua_mmkv_isReadOnly},
#ifndef MMKV_DISABLE_CRYPT
    {"cryptKey", lua_mmkv_cryptKey},
    {"reKey", lua_mmkv_reKey},
#endif
    {nullptr, nullptr}
};

#ifdef __cplusplus
extern "C" {
#endif

int luaopen_mmkv(lua_State *L) {
    luaL_newlib(L, mmkv_functions);
    
    lua_pushinteger(L, MMKV_SINGLE_PROCESS);
    lua_setfield(L, -2, "SINGLE_PROCESS");
    
    lua_pushinteger(L, MMKV_MULTI_PROCESS);
    lua_setfield(L, -2, "MULTI_PROCESS");
    
    lua_pushinteger(L, MMKV_READ_ONLY);
    lua_setfield(L, -2, "READ_ONLY");
    
#ifdef MMKV_ANDROID
    lua_pushinteger(L, CONTEXT_MODE_MULTI_PROCESS);
    lua_setfield(L, -2, "CONTEXT_MODE_MULTI_PROCESS");
    
    lua_pushinteger(L, MMKV_ASHMEM);
    lua_setfield(L, -2, "ASHMEM");
    
    lua_pushinteger(L, MMKV_BACKUP);
    lua_setfield(L, -2, "BACKUP");
#endif
    
    return 1;
}

#ifdef __cplusplus
}
#endif
