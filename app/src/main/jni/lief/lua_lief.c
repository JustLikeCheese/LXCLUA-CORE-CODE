/**
 * @file lua_lief.c
 * @brief LIEF ELF解析库的Lua绑定
 * @description 提供完整的ELF文件解析和修改功能，包括Header、Section、Segment、Symbol等
 * @author DifierLine
 * @date 2026-01-30
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "lief_elf_wrapper.hpp"

/* 模块信息 */
#define MODNAME "liefx"
#define VERSION "1.0.0"

/* Lua userdata类型名称 */
#define LIEF_ELF_BINARY_MT "liefx.elf.binary"

/* ========== 辅助函数 ========== */

/**
 * @brief 从userdata获取ELF Binary Wrapper指针
 * @param L Lua状态机指针
 * @param index 栈索引
 * @return Binary Wrapper指针
 */
static Elf_Binary_Wrapper* check_elf_binary(lua_State *L, int index) {
    Elf_Binary_Wrapper **udata = (Elf_Binary_Wrapper **)luaL_checkudata(L, index, LIEF_ELF_BINARY_MT);
    if (*udata == NULL) {
        luaL_error(L, "ELF binary has been destroyed");
    }
    return *udata;
}

/* ========== 模块级函数 ========== */

/**
 * @brief 解析ELF文件
 * @param L Lua状态机指针
 * @return 返回1表示成功，将ELF Binary userdata推送到栈上
 * @description Lua调用: binary = lief.parse(filepath)
 *              解析指定路径的ELF文件，返回一个可用于后续操作的binary对象
 */
static int lua_elf_parse(lua_State *L) {
    const char *filepath = luaL_checkstring(L, 1);
    
    Elf_Binary_Wrapper *wrapper = lief_elf_parse(filepath);
    if (wrapper == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to parse ELF file");
        return 2;
    }
    
    Elf_Binary_Wrapper **udata = (Elf_Binary_Wrapper **)lua_newuserdata(L, sizeof(Elf_Binary_Wrapper *));
    *udata = wrapper;
    
    luaL_getmetatable(L, LIEF_ELF_BINARY_MT);
    lua_setmetatable(L, -2);
    
    return 1;
}

/**
 * @brief 从内存解析ELF
 * @param L Lua状态机指针
 * @return 返回1或2
 * @description Lua调用: binary = lief.parse_from_memory(data)
 */
static int lua_elf_parse_from_memory(lua_State *L) {
    size_t size;
    const char *data = luaL_checklstring(L, 1, &size);
    
    Elf_Binary_Wrapper *wrapper = lief_elf_parse_from_memory((const uint8_t*)data, size);
    if (wrapper == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to parse ELF from memory");
        return 2;
    }
    
    Elf_Binary_Wrapper **udata = (Elf_Binary_Wrapper **)lua_newuserdata(L, sizeof(Elf_Binary_Wrapper *));
    *udata = wrapper;
    
    luaL_getmetatable(L, LIEF_ELF_BINARY_MT);
    lua_setmetatable(L, -2);
    
    return 1;
}

/**
 * @brief 检查文件是否为ELF格式
 * @param L Lua状态机指针
 * @return 返回1，将布尔值推送到栈上
 * @description Lua调用: is_elf = lief.is_elf(filepath)
 */
static int lua_is_elf(lua_State *L) {
    const char *filepath = luaL_checkstring(L, 1);
    lua_pushboolean(L, lief_is_elf(filepath));
    return 1;
}

/* ========== Binary对象方法 ========== */

/**
 * @brief 写入ELF到文件
 * @param L Lua状态机指针
 * @return 返回1，成功返回true，失败返回nil和错误信息
 * @description Lua调用: binary:write(filepath)
 */
static int lua_elf_write(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *filepath = luaL_checkstring(L, 2);
    
    if (lief_elf_write(wrapper, filepath) == 0) {
        lua_pushboolean(L, 1);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to write ELF file");
        return 2;
    }
}

/**
 * @brief 获取ELF原始数据
 * @param L Lua状态机指针
 * @return 返回1，将原始数据字符串推送到栈上
 * @description Lua调用: data = binary:raw()
 */
static int lua_elf_raw(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    size_t size;
    uint8_t *data = lief_elf_raw(wrapper, &size);
    if (data == NULL) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_pushlstring(L, (const char*)data, size);
    free(data);
    return 1;
}

/**
 * @brief 获取入口点
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: entrypoint = binary:entrypoint()
 */
static int lua_elf_entrypoint(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushinteger(L, (lua_Integer)lief_elf_get_entrypoint(wrapper));
    return 1;
}

/**
 * @brief 设置入口点
 * @param L Lua状态机指针
 * @return 返回0
 * @description Lua调用: binary:set_entrypoint(address)
 */
static int lua_elf_set_entrypoint(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t entrypoint = (uint64_t)luaL_checkinteger(L, 2);
    lief_elf_set_entrypoint(wrapper, entrypoint);
    return 0;
}

/**
 * @brief 获取文件类型
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: type = binary:type()
 */
static int lua_elf_type(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushinteger(L, lief_elf_get_type(wrapper));
    return 1;
}

/**
 * @brief 设置文件类型
 * @param L Lua状态机指针
 * @return 返回0
 * @description Lua调用: binary:set_type(type)
 */
static int lua_elf_set_type(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint32_t type = (uint32_t)luaL_checkinteger(L, 2);
    lief_elf_set_type(wrapper, type);
    return 0;
}

/**
 * @brief 获取机器架构
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: machine = binary:machine()
 */
static int lua_elf_machine(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushinteger(L, lief_elf_get_machine(wrapper));
    return 1;
}

/**
 * @brief 设置机器架构
 * @param L Lua状态机指针
 * @return 返回0
 * @description Lua调用: binary:set_machine(machine)
 */
static int lua_elf_set_machine(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint32_t machine = (uint32_t)luaL_checkinteger(L, 2);
    lief_elf_set_machine(wrapper, machine);
    return 0;
}

/**
 * @brief 获取解释器
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: interp = binary:interpreter()
 */
static int lua_elf_interpreter(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *interp = lief_elf_get_interpreter(wrapper);
    if (interp && strlen(interp) > 0) {
        lua_pushstring(L, interp);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 设置解释器
 * @param L Lua状态机指针
 * @return 返回0
 * @description Lua调用: binary:set_interpreter(path)
 */
static int lua_elf_set_interpreter(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *interp = luaL_checkstring(L, 2);
    lief_elf_set_interpreter(wrapper, interp);
    return 0;
}

/**
 * @brief 检查是否有解释器
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_interpreter()
 */
static int lua_elf_has_interpreter(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushboolean(L, lief_elf_has_interpreter(wrapper));
    return 1;
}

/**
 * @brief 获取所有节信息
 * @param L Lua状态机指针
 * @return 返回1，将节数组推送到栈上
 * @description Lua调用: sections = binary:sections()
 */
static int lua_elf_sections(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    size_t count = lief_elf_sections_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        const char *name = lief_elf_section_name(wrapper, i);
        lua_pushstring(L, name ? name : "");
        lua_setfield(L, -2, "name");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_section_virtual_address(wrapper, i));
        lua_setfield(L, -2, "virtual_address");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_section_size(wrapper, i));
        lua_setfield(L, -2, "size");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_section_offset(wrapper, i));
        lua_setfield(L, -2, "offset");
        
        lua_pushinteger(L, lief_elf_section_type(wrapper, i));
        lua_setfield(L, -2, "type");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_section_flags(wrapper, i));
        lua_setfield(L, -2, "flags");
        
        lua_pushinteger(L, (lua_Integer)(i + 1));
        lua_setfield(L, -2, "index");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    return 1;
}

/**
 * @brief 获取节内容
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: content = binary:section_content(index)
 */
static int lua_elf_section_content(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t index = (size_t)luaL_checkinteger(L, 2) - 1;  /* Lua索引从1开始 */
    
    size_t size;
    const uint8_t *content = lief_elf_section_content(wrapper, index, &size);
    if (content && size > 0) {
        lua_pushlstring(L, (const char*)content, size);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 设置节内容
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:set_section_content(index, content)
 */
static int lua_elf_set_section_content(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t index = (size_t)luaL_checkinteger(L, 2) - 1;
    size_t size;
    const char *content = luaL_checklstring(L, 3, &size);
    
    int result = lief_elf_section_set_content(wrapper, index, (const uint8_t*)content, size);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 添加新节
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: index = binary:add_section(name, type, flags, content, loaded)
 */
static int lua_elf_add_section(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    uint32_t type = (uint32_t)luaL_optinteger(L, 3, 1); /* SHT_PROGBITS */
    uint64_t flags = (uint64_t)luaL_optinteger(L, 4, 0);
    size_t size = 0;
    const char *content = luaL_optlstring(L, 5, NULL, &size);
    int loaded = lua_toboolean(L, 6);
    
    int result = lief_elf_add_section(wrapper, name, type, flags, 
                                      (const uint8_t*)content, size, loaded);
    if (result >= 0) {
        lua_pushinteger(L, result + 1);  /* 转换为Lua索引 */
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 移除节
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:remove_section(name, clear)
 */
static int lua_elf_remove_section(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    int clear = lua_toboolean(L, 3);
    
    int result = lief_elf_remove_section(wrapper, name, clear);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 获取所有段信息
 * @param L Lua状态机指针
 * @return 返回1，将段数组推送到栈上
 * @description Lua调用: segments = binary:segments()
 */
static int lua_elf_segments(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    size_t count = lief_elf_segments_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        lua_pushinteger(L, lief_elf_segment_type(wrapper, i));
        lua_setfield(L, -2, "type");
        
        lua_pushinteger(L, lief_elf_segment_flags(wrapper, i));
        lua_setfield(L, -2, "flags");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_segment_virtual_address(wrapper, i));
        lua_setfield(L, -2, "virtual_address");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_segment_virtual_size(wrapper, i));
        lua_setfield(L, -2, "virtual_size");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_segment_offset(wrapper, i));
        lua_setfield(L, -2, "offset");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_segment_file_size(wrapper, i));
        lua_setfield(L, -2, "file_size");
        
        lua_pushinteger(L, (lua_Integer)(i + 1));
        lua_setfield(L, -2, "index");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    return 1;
}

/**
 * @brief 获取段内容
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: content = binary:segment_content(index)
 */
static int lua_elf_segment_content(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t index = (size_t)luaL_checkinteger(L, 2) - 1;
    
    size_t size;
    const uint8_t *content = lief_elf_segment_content(wrapper, index, &size);
    if (content && size > 0) {
        lua_pushlstring(L, (const char*)content, size);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 设置段内容
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:set_segment_content(index, content)
 */
static int lua_elf_set_segment_content(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t index = (size_t)luaL_checkinteger(L, 2) - 1;
    size_t size;
    const char *content = luaL_checklstring(L, 3, &size);
    
    int result = lief_elf_segment_set_content(wrapper, index, (const uint8_t*)content, size);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 添加新段
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: index = binary:add_segment(type, flags, content, alignment)
 */
static int lua_elf_add_segment(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint32_t type = (uint32_t)luaL_checkinteger(L, 2);
    uint32_t flags = (uint32_t)luaL_optinteger(L, 3, 0);
    size_t size = 0;
    const char *content = luaL_optlstring(L, 4, NULL, &size);
    uint64_t alignment = (uint64_t)luaL_optinteger(L, 5, 0x1000);
    
    int result = lief_elf_add_segment(wrapper, type, flags, 
                                      (const uint8_t*)content, size, alignment);
    if (result >= 0) {
        lua_pushinteger(L, result + 1);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 获取所有动态符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: symbols = binary:dynamic_symbols()
 */
static int lua_elf_dynamic_symbols(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    size_t count = lief_elf_dynamic_symbols_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        const char *name = lief_elf_dynamic_symbol_name(wrapper, i);
        lua_pushstring(L, name ? name : "");
        lua_setfield(L, -2, "name");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_dynamic_symbol_value(wrapper, i));
        lua_setfield(L, -2, "value");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_dynamic_symbol_size(wrapper, i));
        lua_setfield(L, -2, "size");
        
        lua_pushinteger(L, lief_elf_dynamic_symbol_type(wrapper, i));
        lua_setfield(L, -2, "type");
        
        lua_pushinteger(L, lief_elf_dynamic_symbol_binding(wrapper, i));
        lua_setfield(L, -2, "binding");
        
        lua_pushinteger(L, (lua_Integer)(i + 1));
        lua_setfield(L, -2, "index");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    return 1;
}

/**
 * @brief 添加动态符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: index = binary:add_dynamic_symbol(name, value, size, type, binding)
 */
static int lua_elf_add_dynamic_symbol(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    uint64_t value = (uint64_t)luaL_optinteger(L, 3, 0);
    uint64_t size = (uint64_t)luaL_optinteger(L, 4, 0);
    uint32_t type = (uint32_t)luaL_optinteger(L, 5, 0);  /* STT_NOTYPE */
    uint32_t binding = (uint32_t)luaL_optinteger(L, 6, 1);  /* STB_GLOBAL */
    
    int result = lief_elf_add_dynamic_symbol(wrapper, name, value, size, type, binding);
    if (result >= 0) {
        lua_pushinteger(L, result + 1);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 移除动态符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:remove_dynamic_symbol(name)
 */
static int lua_elf_remove_dynamic_symbol(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    
    int result = lief_elf_remove_dynamic_symbol(wrapper, name);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 导出符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:export_symbol(name, value)
 */
static int lua_elf_export_symbol(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    uint64_t value = (uint64_t)luaL_optinteger(L, 3, 0);
    
    int result = lief_elf_export_symbol(wrapper, name, value);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 获取所有动态条目
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: entries = binary:dynamic_entries()
 */
static int lua_elf_dynamic_entries(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    size_t count = lief_elf_dynamic_entries_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        lua_pushinteger(L, (lua_Integer)lief_elf_dynamic_entry_tag(wrapper, i));
        lua_setfield(L, -2, "tag");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_dynamic_entry_value(wrapper, i));
        lua_setfield(L, -2, "value");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    return 1;
}

/**
 * @brief 移除动态条目
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:remove_dynamic_entry(tag)
 */
static int lua_elf_remove_dynamic_entry(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t tag = (uint64_t)luaL_checkinteger(L, 2);
    
    int result = lief_elf_remove_dynamic_entry(wrapper, tag);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 添加库依赖
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:add_library(name)
 */
static int lua_elf_add_library(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    
    int result = lief_elf_add_library(wrapper, name);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 移除库依赖
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:remove_library(name)
 */
static int lua_elf_remove_library(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    
    int result = lief_elf_remove_library(wrapper, name);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 检查库依赖
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_library(name)
 */
static int lua_elf_has_library(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    
    lua_pushboolean(L, lief_elf_has_library(wrapper, name));
    return 1;
}

/**
 * @brief 在虚拟地址处打补丁（字节数据）
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:patch(address, data)
 */
static int lua_elf_patch(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t address = (uint64_t)luaL_checkinteger(L, 2);
    size_t size;
    const char *data = luaL_checklstring(L, 3, &size);
    
    int result = lief_elf_patch_address(wrapper, address, (const uint8_t*)data, size);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 在虚拟地址处打补丁（整数值）
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:patch_value(address, value, size)
 */
static int lua_elf_patch_value(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t address = (uint64_t)luaL_checkinteger(L, 2);
    uint64_t value = (uint64_t)luaL_checkinteger(L, 3);
    size_t size = (size_t)luaL_optinteger(L, 4, 8);
    
    int result = lief_elf_patch_address_value(wrapper, address, value, size);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 修补PLT/GOT
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:patch_pltgot(symbol_name, address)
 */
static int lua_elf_patch_pltgot(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *symbol_name = luaL_checkstring(L, 2);
    uint64_t address = (uint64_t)luaL_checkinteger(L, 3);
    
    int result = lief_elf_patch_pltgot(wrapper, symbol_name, address);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief Strip二进制
 * @param L Lua状态机指针
 * @return 返回0
 * @description Lua调用: binary:strip()
 */
static int lua_elf_strip(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lief_elf_strip(wrapper);
    return 0;
}

/**
 * @brief 检查是否为PIE
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: is_pie = binary:is_pie()
 */
static int lua_elf_is_pie(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushboolean(L, lief_elf_is_pie(wrapper));
    return 1;
}

/**
 * @brief 检查是否有NX保护
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has_nx = binary:has_nx()
 */
static int lua_elf_has_nx(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushboolean(L, lief_elf_has_nx(wrapper));
    return 1;
}

/**
 * @brief 获取image base
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: base = binary:imagebase()
 */
static int lua_elf_imagebase(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushinteger(L, (lua_Integer)lief_elf_imagebase(wrapper));
    return 1;
}

/**
 * @brief 虚拟地址转文件偏移
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: offset = binary:va_to_offset(va)
 */
static int lua_elf_va_to_offset(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t va = (uint64_t)luaL_checkinteger(L, 2);
    
    uint64_t offset;
    if (lief_elf_va_to_offset(wrapper, va, &offset) == 0) {
        lua_pushinteger(L, (lua_Integer)offset);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 文件偏移转虚拟地址
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: va = binary:offset_to_va(offset)
 */
static int lua_elf_offset_to_va(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t offset = (uint64_t)luaL_checkinteger(L, 2);
    
    uint64_t va;
    if (lief_elf_offset_to_va(wrapper, offset, &va) == 0) {
        lua_pushinteger(L, (lua_Integer)va);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 获取所有重定位
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: relocs = binary:relocations()
 */
static int lua_elf_relocations(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    size_t count = lief_elf_relocations_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        lua_pushinteger(L, (lua_Integer)lief_elf_relocation_address(wrapper, i));
        lua_setfield(L, -2, "address");
        
        lua_pushinteger(L, lief_elf_relocation_type(wrapper, i));
        lua_setfield(L, -2, "type");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_relocation_addend(wrapper, i));
        lua_setfield(L, -2, "addend");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    return 1;
}

/* ========== 扩展Section Lua绑定 ========== */

/**
 * @brief 获取节（通过名称）
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: section = binary:get_section(name)
 */
static int lua_elf_get_section(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    
    int idx = lief_elf_get_section_index(wrapper, name);
    if (idx < 0) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_newtable(L);
    lua_pushstring(L, lief_elf_section_name(wrapper, idx));
    lua_setfield(L, -2, "name");
    lua_pushinteger(L, (lua_Integer)lief_elf_section_virtual_address(wrapper, idx));
    lua_setfield(L, -2, "virtual_address");
    lua_pushinteger(L, (lua_Integer)lief_elf_section_size(wrapper, idx));
    lua_setfield(L, -2, "size");
    lua_pushinteger(L, (lua_Integer)lief_elf_section_offset(wrapper, idx));
    lua_setfield(L, -2, "offset");
    lua_pushinteger(L, lief_elf_section_type(wrapper, idx));
    lua_setfield(L, -2, "type");
    lua_pushinteger(L, (lua_Integer)lief_elf_section_flags(wrapper, idx));
    lua_setfield(L, -2, "flags");
    lua_pushinteger(L, (lua_Integer)lief_elf_section_alignment(wrapper, idx));
    lua_setfield(L, -2, "alignment");
    lua_pushinteger(L, (lua_Integer)lief_elf_section_entry_size(wrapper, idx));
    lua_setfield(L, -2, "entry_size");
    lua_pushinteger(L, lief_elf_section_info(wrapper, idx));
    lua_setfield(L, -2, "info");
    lua_pushinteger(L, lief_elf_section_link(wrapper, idx));
    lua_setfield(L, -2, "link");
    lua_pushinteger(L, idx + 1);
    lua_setfield(L, -2, "index");
    
    return 1;
}

/**
 * @brief 检查是否有指定名称的节
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_section(name)
 */
static int lua_elf_has_section(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    lua_pushboolean(L, lief_elf_has_section(wrapper, name));
    return 1;
}

/**
 * @brief 修改节属性
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:modify_section(index, {type=..., flags=..., ...})
 */
static int lua_elf_modify_section(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t index = (size_t)luaL_checkinteger(L, 2) - 1;
    luaL_checktype(L, 3, LUA_TTABLE);
    
    /* 检查并设置各个属性 */
    lua_getfield(L, 3, "type");
    if (!lua_isnil(L, -1)) {
        lief_elf_section_set_type(wrapper, index, (uint32_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "flags");
    if (!lua_isnil(L, -1)) {
        lief_elf_section_set_flags(wrapper, index, (uint64_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "virtual_address");
    if (!lua_isnil(L, -1)) {
        lief_elf_section_set_virtual_address(wrapper, index, (uint64_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "alignment");
    if (!lua_isnil(L, -1)) {
        lief_elf_section_set_alignment(wrapper, index, (uint64_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "entry_size");
    if (!lua_isnil(L, -1)) {
        lief_elf_section_set_entry_size(wrapper, index, (uint64_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "info");
    if (!lua_isnil(L, -1)) {
        lief_elf_section_set_info(wrapper, index, (uint32_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "link");
    if (!lua_isnil(L, -1)) {
        lief_elf_section_set_link(wrapper, index, (uint32_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_pushboolean(L, 1);
    return 1;
}

/* ========== 扩展Segment Lua绑定 ========== */

/**
 * @brief 获取段（通过类型）
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: segment = binary:get_segment(type)
 */
static int lua_elf_get_segment(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint32_t type = (uint32_t)luaL_checkinteger(L, 2);
    
    int idx = lief_elf_get_segment_index(wrapper, type);
    if (idx < 0) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_newtable(L);
    lua_pushinteger(L, lief_elf_segment_type(wrapper, idx));
    lua_setfield(L, -2, "type");
    lua_pushinteger(L, lief_elf_segment_flags(wrapper, idx));
    lua_setfield(L, -2, "flags");
    lua_pushinteger(L, (lua_Integer)lief_elf_segment_virtual_address(wrapper, idx));
    lua_setfield(L, -2, "virtual_address");
    lua_pushinteger(L, (lua_Integer)lief_elf_segment_physical_address(wrapper, idx));
    lua_setfield(L, -2, "physical_address");
    lua_pushinteger(L, (lua_Integer)lief_elf_segment_virtual_size(wrapper, idx));
    lua_setfield(L, -2, "virtual_size");
    lua_pushinteger(L, (lua_Integer)lief_elf_segment_file_size(wrapper, idx));
    lua_setfield(L, -2, "file_size");
    lua_pushinteger(L, (lua_Integer)lief_elf_segment_offset(wrapper, idx));
    lua_setfield(L, -2, "offset");
    lua_pushinteger(L, (lua_Integer)lief_elf_segment_alignment(wrapper, idx));
    lua_setfield(L, -2, "alignment");
    lua_pushinteger(L, idx + 1);
    lua_setfield(L, -2, "index");
    
    return 1;
}

/**
 * @brief 检查是否有指定类型的段
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_segment(type)
 */
static int lua_elf_has_segment(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint32_t type = (uint32_t)luaL_checkinteger(L, 2);
    lua_pushboolean(L, lief_elf_has_segment(wrapper, type));
    return 1;
}

/**
 * @brief 修改段属性
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:modify_segment(index, {type=..., flags=..., ...})
 */
static int lua_elf_modify_segment(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t index = (size_t)luaL_checkinteger(L, 2) - 1;
    luaL_checktype(L, 3, LUA_TTABLE);
    
    lua_getfield(L, 3, "type");
    if (!lua_isnil(L, -1)) {
        lief_elf_segment_set_type(wrapper, index, (uint32_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "flags");
    if (!lua_isnil(L, -1)) {
        lief_elf_segment_set_flags(wrapper, index, (uint32_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "virtual_address");
    if (!lua_isnil(L, -1)) {
        lief_elf_segment_set_virtual_address(wrapper, index, (uint64_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "physical_address");
    if (!lua_isnil(L, -1)) {
        lief_elf_segment_set_physical_address(wrapper, index, (uint64_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "virtual_size");
    if (!lua_isnil(L, -1)) {
        lief_elf_segment_set_virtual_size(wrapper, index, (uint64_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "physical_size");
    if (!lua_isnil(L, -1)) {
        lief_elf_segment_set_physical_size(wrapper, index, (uint64_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "file_offset");
    if (!lua_isnil(L, -1)) {
        lief_elf_segment_set_file_offset(wrapper, index, (uint64_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_getfield(L, 3, "alignment");
    if (!lua_isnil(L, -1)) {
        lief_elf_segment_set_alignment(wrapper, index, (uint64_t)lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
    
    lua_pushboolean(L, 1);
    return 1;
}

/**
 * @brief 移除段
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:remove_segment(index, clear)
 */
static int lua_elf_remove_segment(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t index = (size_t)luaL_checkinteger(L, 2) - 1;
    int clear = lua_toboolean(L, 3);
    
    int result = lief_elf_remove_segment(wrapper, index, clear);
    lua_pushboolean(L, result == 0);
    return 1;
}

/* ========== symtab符号 Lua绑定 ========== */

/**
 * @brief 获取所有symtab符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: symbols = binary:symtab_symbols()
 */
static int lua_elf_symtab_symbols(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    size_t count = lief_elf_symtab_symbols_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        const char *name = lief_elf_symtab_symbol_name(wrapper, i);
        lua_pushstring(L, name ? name : "");
        lua_setfield(L, -2, "name");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_symtab_symbol_value(wrapper, i));
        lua_setfield(L, -2, "value");
        
        lua_pushinteger(L, (lua_Integer)lief_elf_symtab_symbol_size(wrapper, i));
        lua_setfield(L, -2, "size");
        
        lua_pushinteger(L, lief_elf_symtab_symbol_type(wrapper, i));
        lua_setfield(L, -2, "type");
        
        lua_pushinteger(L, lief_elf_symtab_symbol_binding(wrapper, i));
        lua_setfield(L, -2, "binding");
        
        lua_pushinteger(L, (lua_Integer)(i + 1));
        lua_setfield(L, -2, "index");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    return 1;
}

/**
 * @brief 添加symtab符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: index = binary:add_symtab_symbol(name, value, size, type, binding)
 */
static int lua_elf_add_symtab_symbol(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    uint64_t value = (uint64_t)luaL_optinteger(L, 3, 0);
    uint64_t size = (uint64_t)luaL_optinteger(L, 4, 0);
    uint32_t type = (uint32_t)luaL_optinteger(L, 5, 0);
    uint32_t binding = (uint32_t)luaL_optinteger(L, 6, 0);
    
    int result = lief_elf_add_symtab_symbol(wrapper, name, value, size, type, binding);
    if (result >= 0) {
        lua_pushinteger(L, result + 1);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 移除symtab符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:remove_symtab_symbol(name)
 */
static int lua_elf_remove_symtab_symbol(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    
    int result = lief_elf_remove_symtab_symbol(wrapper, name);
    lua_pushboolean(L, result == 0);
    return 1;
}

/* ========== 扩展信息功能 Lua绑定 ========== */

/**
 * @brief 获取虚拟大小
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: size = binary:virtual_size()
 */
static int lua_elf_virtual_size(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushinteger(L, (lua_Integer)lief_elf_virtual_size(wrapper));
    return 1;
}

/**
 * @brief 获取EOF偏移
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: offset = binary:eof_offset()
 */
static int lua_elf_eof_offset(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushinteger(L, (lua_Integer)lief_elf_eof_offset(wrapper));
    return 1;
}

/**
 * @brief 检查是否针对Android
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: is_android = binary:is_targeting_android()
 */
static int lua_elf_is_targeting_android(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushboolean(L, lief_elf_is_targeting_android(wrapper));
    return 1;
}

/**
 * @brief 从虚拟地址读取内容
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: data = binary:read_from_va(va, size)
 */
static int lua_elf_read_from_va(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t va = (uint64_t)luaL_checkinteger(L, 2);
    uint64_t size = (uint64_t)luaL_checkinteger(L, 3);
    
    size_t out_size;
    const uint8_t *data = lief_elf_get_content_from_va(wrapper, va, size, &out_size);
    if (data && out_size > 0) {
        lua_pushlstring(L, (const char*)data, out_size);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 从偏移获取节
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: index = binary:section_from_offset(offset)
 */
static int lua_elf_section_from_offset(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t offset = (uint64_t)luaL_checkinteger(L, 2);
    
    int idx = lief_elf_section_from_offset(wrapper, offset);
    if (idx >= 0) {
        lua_pushinteger(L, idx + 1);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 从虚拟地址获取节
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: index = binary:section_from_va(va)
 */
static int lua_elf_section_from_va(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t va = (uint64_t)luaL_checkinteger(L, 2);
    
    int idx = lief_elf_section_from_va(wrapper, va);
    if (idx >= 0) {
        lua_pushinteger(L, idx + 1);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 从偏移获取段
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: index = binary:segment_from_offset(offset)
 */
static int lua_elf_segment_from_offset(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t offset = (uint64_t)luaL_checkinteger(L, 2);
    
    int idx = lief_elf_segment_from_offset(wrapper, offset);
    if (idx >= 0) {
        lua_pushinteger(L, idx + 1);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 从虚拟地址获取段
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: index = binary:segment_from_va(va)
 */
static int lua_elf_segment_from_va(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t va = (uint64_t)luaL_checkinteger(L, 2);
    
    int idx = lief_elf_segment_from_va(wrapper, va);
    if (idx >= 0) {
        lua_pushinteger(L, idx + 1);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* ========== Overlay Lua绑定 ========== */

/**
 * @brief 检查是否有overlay
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_overlay()
 */
static int lua_elf_has_overlay(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushboolean(L, lief_elf_has_overlay(wrapper));
    return 1;
}

/**
 * @brief 获取overlay数据
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: data = binary:overlay()
 */
static int lua_elf_overlay(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    size_t size;
    const uint8_t *data = lief_elf_get_overlay(wrapper, &size);
    if (data && size > 0) {
        lua_pushlstring(L, (const char*)data, size);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 设置overlay数据
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:set_overlay(data)
 */
static int lua_elf_set_overlay(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t size;
    const char *data = luaL_optlstring(L, 2, NULL, &size);
    
    int result = lief_elf_set_overlay(wrapper, (const uint8_t*)data, size);
    lua_pushboolean(L, result == 0);
    return 1;
}

/* ========== 扩展动态条目 Lua绑定 ========== */

/**
 * @brief 检查是否有指定标签的动态条目
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_dynamic_entry(tag)
 */
static int lua_elf_has_dynamic_entry(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t tag = (uint64_t)luaL_checkinteger(L, 2);
    lua_pushboolean(L, lief_elf_has_dynamic_entry(wrapper, tag));
    return 1;
}

/**
 * @brief 获取指定标签的动态条目值
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: value = binary:get_dynamic_entry(tag)
 */
static int lua_elf_get_dynamic_entry(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t tag = (uint64_t)luaL_checkinteger(L, 2);
    
    uint64_t value;
    if (lief_elf_get_dynamic_entry_by_tag(wrapper, tag, &value) == 0) {
        lua_pushinteger(L, (lua_Integer)value);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 添加动态重定位
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:add_dynamic_relocation(address, type, addend, symbol_name)
 */
static int lua_elf_add_dynamic_relocation(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t address = (uint64_t)luaL_checkinteger(L, 2);
    uint32_t type = (uint32_t)luaL_checkinteger(L, 3);
    int64_t addend = (int64_t)luaL_optinteger(L, 4, 0);
    const char *symbol_name = luaL_optstring(L, 5, NULL);
    
    int result = lief_elf_add_dynamic_relocation(wrapper, address, type, addend, symbol_name);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 添加PLT/GOT重定位
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: binary:add_pltgot_relocation(address, type, symbol_name)
 */
static int lua_elf_add_pltgot_relocation(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t address = (uint64_t)luaL_checkinteger(L, 2);
    uint32_t type = (uint32_t)luaL_checkinteger(L, 3);
    const char *symbol_name = luaL_checkstring(L, 4);
    
    int result = lief_elf_add_pltgot_relocation(wrapper, address, type, symbol_name);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 销毁ELF Binary对象
 * @param L Lua状态机指针
 * @return 返回0
 * @description 垃圾回收时自动调用，也可手动调用: binary:destroy()
 */
static int lua_elf_destroy(lua_State *L) {
    Elf_Binary_Wrapper **udata = (Elf_Binary_Wrapper **)luaL_checkudata(L, 1, LIEF_ELF_BINARY_MT);
    if (*udata) {
        lief_elf_destroy(*udata);
        *udata = NULL;
    }
    return 0;
}

/**
 * @brief ELF Binary对象的__tostring元方法
 * @param L Lua状态机指针
 * @return 返回1
 */
static int lua_elf_tostring(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    uint32_t type = lief_elf_get_type(wrapper);
    uint32_t machine = lief_elf_get_machine(wrapper);
    
    const char *type_str;
    switch (type) {
        case 0: type_str = "NONE"; break;
        case 1: type_str = "REL"; break;
        case 2: type_str = "EXEC"; break;
        case 3: type_str = "DYN"; break;
        case 4: type_str = "CORE"; break;
        default: type_str = "UNKNOWN"; break;
    }
    
    const char *arch_str;
    switch (machine) {
        case 3:   arch_str = "i386"; break;
        case 62:  arch_str = "x86_64"; break;
        case 40:  arch_str = "ARM"; break;
        case 183: arch_str = "AArch64"; break;
        case 8:   arch_str = "MIPS"; break;
        case 243: arch_str = "RISC-V"; break;
        default:  arch_str = "Unknown"; break;
    }
    
    size_t sections_count = lief_elf_sections_count(wrapper);
    size_t segments_count = lief_elf_segments_count(wrapper);
    
    lua_pushfstring(L, "ELF Binary [%s, %s, %d sections, %d segments]",
                    type_str, arch_str,
                    (int)sections_count, (int)segments_count);
    return 1;
}

/* ELF Binary对象的方法表 */
static const luaL_Reg elf_binary_methods[] = {
    /* 文件操作 */
    {"write",                lua_elf_write},
    {"raw",                  lua_elf_raw},
    {"destroy",              lua_elf_destroy},
    
    /* Header操作 */
    {"entrypoint",           lua_elf_entrypoint},
    {"set_entrypoint",       lua_elf_set_entrypoint},
    {"type",                 lua_elf_type},
    {"set_type",             lua_elf_set_type},
    {"machine",              lua_elf_machine},
    {"set_machine",          lua_elf_set_machine},
    
    /* 解释器操作 */
    {"interpreter",          lua_elf_interpreter},
    {"set_interpreter",      lua_elf_set_interpreter},
    {"has_interpreter",      lua_elf_has_interpreter},
    
    /* Section操作 */
    {"sections",             lua_elf_sections},
    {"section_content",      lua_elf_section_content},
    {"set_section_content",  lua_elf_set_section_content},
    {"add_section",          lua_elf_add_section},
    {"remove_section",       lua_elf_remove_section},
    {"get_section",          lua_elf_get_section},
    {"has_section",          lua_elf_has_section},
    {"modify_section",       lua_elf_modify_section},
    {"section_from_offset",  lua_elf_section_from_offset},
    {"section_from_va",      lua_elf_section_from_va},
    
    /* Segment操作 */
    {"segments",             lua_elf_segments},
    {"segment_content",      lua_elf_segment_content},
    {"set_segment_content",  lua_elf_set_segment_content},
    {"add_segment",          lua_elf_add_segment},
    {"remove_segment",       lua_elf_remove_segment},
    {"get_segment",          lua_elf_get_segment},
    {"has_segment",          lua_elf_has_segment},
    {"modify_segment",       lua_elf_modify_segment},
    {"segment_from_offset",  lua_elf_segment_from_offset},
    {"segment_from_va",      lua_elf_segment_from_va},
    
    /* 动态符号操作 */
    {"dynamic_symbols",      lua_elf_dynamic_symbols},
    {"add_dynamic_symbol",   lua_elf_add_dynamic_symbol},
    {"remove_dynamic_symbol",lua_elf_remove_dynamic_symbol},
    {"export_symbol",        lua_elf_export_symbol},
    
    /* symtab符号操作 */
    {"symtab_symbols",       lua_elf_symtab_symbols},
    {"add_symtab_symbol",    lua_elf_add_symtab_symbol},
    {"remove_symtab_symbol", lua_elf_remove_symtab_symbol},
    
    /* 动态条目操作 */
    {"dynamic_entries",      lua_elf_dynamic_entries},
    {"remove_dynamic_entry", lua_elf_remove_dynamic_entry},
    {"has_dynamic_entry",    lua_elf_has_dynamic_entry},
    {"get_dynamic_entry",    lua_elf_get_dynamic_entry},
    
    /* 库依赖操作 */
    {"add_library",          lua_elf_add_library},
    {"remove_library",       lua_elf_remove_library},
    {"has_library",          lua_elf_has_library},
    
    /* Patch操作 */
    {"patch",                lua_elf_patch},
    {"patch_value",          lua_elf_patch_value},
    {"patch_pltgot",         lua_elf_patch_pltgot},
    
    /* 重定位 */
    {"relocations",          lua_elf_relocations},
    {"add_dynamic_relocation", lua_elf_add_dynamic_relocation},
    {"add_pltgot_relocation", lua_elf_add_pltgot_relocation},
    
    /* 内容读取 */
    {"read_from_va",         lua_elf_read_from_va},
    
    /* Overlay */
    {"has_overlay",          lua_elf_has_overlay},
    {"overlay",              lua_elf_overlay},
    {"set_overlay",          lua_elf_set_overlay},
    
    /* 其他功能 */
    {"strip",                lua_elf_strip},
    {"is_pie",               lua_elf_is_pie},
    {"has_nx",               lua_elf_has_nx},
    {"imagebase",            lua_elf_imagebase},
    {"virtual_size",         lua_elf_virtual_size},
    {"eof_offset",           lua_elf_eof_offset},
    {"is_targeting_android", lua_elf_is_targeting_android},
    {"va_to_offset",         lua_elf_va_to_offset},
    {"offset_to_va",         lua_elf_offset_to_va},
    
    /* 元方法 */
    {"__gc",                 lua_elf_destroy},
    {"__tostring",           lua_elf_tostring},
    {NULL, NULL}
};

/* 模块级函数表 */
static const luaL_Reg lief_funcs[] = {
    {"parse",             lua_elf_parse},
    {"parse_from_memory", lua_elf_parse_from_memory},
    {"is_elf",            lua_is_elf},
    {NULL, NULL}
};

/**
 * @brief 注册ELF常量到Lua表
 * @param L Lua状态机指针
 */
static void register_elf_constants(lua_State *L) {
    /* ELF文件类型常量 */
    lua_newtable(L);
    lua_pushinteger(L, 0); lua_setfield(L, -2, "NONE");
    lua_pushinteger(L, 1); lua_setfield(L, -2, "REL");
    lua_pushinteger(L, 2); lua_setfield(L, -2, "EXEC");
    lua_pushinteger(L, 3); lua_setfield(L, -2, "DYN");
    lua_pushinteger(L, 4); lua_setfield(L, -2, "CORE");
    lua_setfield(L, -2, "E_TYPE");
    
    /* 机器架构常量 */
    lua_newtable(L);
    lua_pushinteger(L, 0);   lua_setfield(L, -2, "NONE");
    lua_pushinteger(L, 3);   lua_setfield(L, -2, "I386");
    lua_pushinteger(L, 62);  lua_setfield(L, -2, "X86_64");
    lua_pushinteger(L, 40);  lua_setfield(L, -2, "ARM");
    lua_pushinteger(L, 183); lua_setfield(L, -2, "AARCH64");
    lua_pushinteger(L, 8);   lua_setfield(L, -2, "MIPS");
    lua_pushinteger(L, 243); lua_setfield(L, -2, "RISCV");
    lua_pushinteger(L, 20);  lua_setfield(L, -2, "PPC");
    lua_pushinteger(L, 21);  lua_setfield(L, -2, "PPC64");
    lua_setfield(L, -2, "ARCH");
    
    /* 节类型常量 */
    lua_newtable(L);
    lua_pushinteger(L, 0);  lua_setfield(L, -2, "NULL");
    lua_pushinteger(L, 1);  lua_setfield(L, -2, "PROGBITS");
    lua_pushinteger(L, 2);  lua_setfield(L, -2, "SYMTAB");
    lua_pushinteger(L, 3);  lua_setfield(L, -2, "STRTAB");
    lua_pushinteger(L, 4);  lua_setfield(L, -2, "RELA");
    lua_pushinteger(L, 5);  lua_setfield(L, -2, "HASH");
    lua_pushinteger(L, 6);  lua_setfield(L, -2, "DYNAMIC");
    lua_pushinteger(L, 7);  lua_setfield(L, -2, "NOTE");
    lua_pushinteger(L, 8);  lua_setfield(L, -2, "NOBITS");
    lua_pushinteger(L, 9);  lua_setfield(L, -2, "REL");
    lua_pushinteger(L, 11); lua_setfield(L, -2, "DYNSYM");
    lua_pushinteger(L, 14); lua_setfield(L, -2, "INIT_ARRAY");
    lua_pushinteger(L, 15); lua_setfield(L, -2, "FINI_ARRAY");
    lua_setfield(L, -2, "SHT");
    
    /* 节标志常量 */
    lua_newtable(L);
    lua_pushinteger(L, 0x0); lua_setfield(L, -2, "NONE");
    lua_pushinteger(L, 0x1); lua_setfield(L, -2, "WRITE");
    lua_pushinteger(L, 0x2); lua_setfield(L, -2, "ALLOC");
    lua_pushinteger(L, 0x4); lua_setfield(L, -2, "EXECINSTR");
    lua_pushinteger(L, 0x10); lua_setfield(L, -2, "MERGE");
    lua_pushinteger(L, 0x20); lua_setfield(L, -2, "STRINGS");
    lua_pushinteger(L, 0x400); lua_setfield(L, -2, "TLS");
    lua_setfield(L, -2, "SHF");
    
    /* 段类型常量 */
    lua_newtable(L);
    lua_pushinteger(L, 0); lua_setfield(L, -2, "NULL");
    lua_pushinteger(L, 1); lua_setfield(L, -2, "LOAD");
    lua_pushinteger(L, 2); lua_setfield(L, -2, "DYNAMIC");
    lua_pushinteger(L, 3); lua_setfield(L, -2, "INTERP");
    lua_pushinteger(L, 4); lua_setfield(L, -2, "NOTE");
    lua_pushinteger(L, 5); lua_setfield(L, -2, "SHLIB");
    lua_pushinteger(L, 6); lua_setfield(L, -2, "PHDR");
    lua_pushinteger(L, 7); lua_setfield(L, -2, "TLS");
    lua_pushinteger(L, 0x6474e550); lua_setfield(L, -2, "GNU_EH_FRAME");
    lua_pushinteger(L, 0x6474e551); lua_setfield(L, -2, "GNU_STACK");
    lua_pushinteger(L, 0x6474e552); lua_setfield(L, -2, "GNU_RELRO");
    lua_setfield(L, -2, "PT");
    
    /* 段标志常量 */
    lua_newtable(L);
    lua_pushinteger(L, 0); lua_setfield(L, -2, "NONE");
    lua_pushinteger(L, 1); lua_setfield(L, -2, "X");
    lua_pushinteger(L, 2); lua_setfield(L, -2, "W");
    lua_pushinteger(L, 4); lua_setfield(L, -2, "R");
    lua_setfield(L, -2, "PF");
    
    /* 符号绑定常量 */
    lua_newtable(L);
    lua_pushinteger(L, 0);  lua_setfield(L, -2, "LOCAL");
    lua_pushinteger(L, 1);  lua_setfield(L, -2, "GLOBAL");
    lua_pushinteger(L, 2);  lua_setfield(L, -2, "WEAK");
    lua_pushinteger(L, 10); lua_setfield(L, -2, "GNU_UNIQUE");
    lua_setfield(L, -2, "STB");
    
    /* 符号类型常量 */
    lua_newtable(L);
    lua_pushinteger(L, 0);  lua_setfield(L, -2, "NOTYPE");
    lua_pushinteger(L, 1);  lua_setfield(L, -2, "OBJECT");
    lua_pushinteger(L, 2);  lua_setfield(L, -2, "FUNC");
    lua_pushinteger(L, 3);  lua_setfield(L, -2, "SECTION");
    lua_pushinteger(L, 4);  lua_setfield(L, -2, "FILE");
    lua_pushinteger(L, 5);  lua_setfield(L, -2, "COMMON");
    lua_pushinteger(L, 6);  lua_setfield(L, -2, "TLS");
    lua_pushinteger(L, 10); lua_setfield(L, -2, "GNU_IFUNC");
    lua_setfield(L, -2, "STT");
    
    /* 动态标签常量 */
    lua_newtable(L);
    lua_pushinteger(L, 0);  lua_setfield(L, -2, "NULL");
    lua_pushinteger(L, 1);  lua_setfield(L, -2, "NEEDED");
    lua_pushinteger(L, 2);  lua_setfield(L, -2, "PLTRELSZ");
    lua_pushinteger(L, 3);  lua_setfield(L, -2, "PLTGOT");
    lua_pushinteger(L, 4);  lua_setfield(L, -2, "HASH");
    lua_pushinteger(L, 5);  lua_setfield(L, -2, "STRTAB");
    lua_pushinteger(L, 6);  lua_setfield(L, -2, "SYMTAB");
    lua_pushinteger(L, 7);  lua_setfield(L, -2, "RELA");
    lua_pushinteger(L, 12); lua_setfield(L, -2, "INIT");
    lua_pushinteger(L, 13); lua_setfield(L, -2, "FINI");
    lua_pushinteger(L, 14); lua_setfield(L, -2, "SONAME");
    lua_pushinteger(L, 15); lua_setfield(L, -2, "RPATH");
    lua_pushinteger(L, 21); lua_setfield(L, -2, "DEBUG");
    lua_pushinteger(L, 23); lua_setfield(L, -2, "JMPREL");
    lua_pushinteger(L, 25); lua_setfield(L, -2, "INIT_ARRAY");
    lua_pushinteger(L, 26); lua_setfield(L, -2, "FINI_ARRAY");
    lua_pushinteger(L, 29); lua_setfield(L, -2, "RUNPATH");
    lua_pushinteger(L, 30); lua_setfield(L, -2, "FLAGS");
    lua_pushinteger(L, 0x6FFFFFFB); lua_setfield(L, -2, "FLAGS_1");
    lua_pushinteger(L, 0x6FFFFEF5); lua_setfield(L, -2, "GNU_HASH");
    lua_setfield(L, -2, "DT");
}

/**
 * @brief 模块入口函数
 * @param L Lua状态机指针
 * @return 返回1，将模块表推送到栈上
 */
int luaopen_liefx(lua_State *L) {
    /* 创建ELF Binary的metatable */
    luaL_newmetatable(L, LIEF_ELF_BINARY_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, elf_binary_methods, 0);
    lua_pop(L, 1);
    
    /* 创建模块表 */
    luaL_newlib(L, lief_funcs);
    
    /* 设置模块信息 */
    lua_pushliteral(L, MODNAME);
    lua_setfield(L, -2, "_NAME");
    
    lua_pushliteral(L, VERSION);
    lua_setfield(L, -2, "_VERSION");
    
    /* 注册所有ELF常量 */
    register_elf_constants(L);
    
    return 1;
}
