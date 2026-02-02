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

/**
 * @brief 将uint64地址推送到Lua栈（使用lua_Number保证大数精度）
 * @param L Lua状态机指针
 * @param addr 64位地址值
 * @description 使用lua_pushnumber确保大于32位的地址能正确表示
 */
static void push_address(lua_State *L, uint64_t addr) {
    lua_pushnumber(L, (lua_Number)addr);
}

/**
 * @brief 从Lua栈获取uint64地址
 * @param L Lua状态机指针
 * @param index 栈索引
 * @return 64位地址值
 * @description 支持两种输入格式:
 *              - 数字: 直接传入数值 如 0x1000 或 4096
 *              - 字符串: 十六进制字符串 如 "0x1000" 或 "1000"
 */
static uint64_t check_address(lua_State *L, int index) {
    if (lua_type(L, index) == LUA_TSTRING) {
        /* 支持字符串形式的十六进制地址 */
        const char *str = lua_tostring(L, index);
        uint64_t addr = 0;
        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            /* 0x前缀的十六进制 */
            sscanf(str + 2, "%llx", (unsigned long long*)&addr);
        } else {
            /* 尝试解析为十六进制 */
            sscanf(str, "%llx", (unsigned long long*)&addr);
        }
        return addr;
    }
    return (uint64_t)luaL_checknumber(L, index);
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
    push_address(L, lief_elf_get_entrypoint(wrapper));
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
    uint64_t entrypoint = check_address(L, 2);
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
        
        push_address(L, lief_elf_section_virtual_address(wrapper, i));
        lua_setfield(L, -2, "virtual_address");
        
        push_address(L, lief_elf_section_size(wrapper, i));
        lua_setfield(L, -2, "size");
        
        push_address(L, lief_elf_section_offset(wrapper, i));
        lua_setfield(L, -2, "offset");
        
        lua_pushinteger(L, lief_elf_section_type(wrapper, i));
        lua_setfield(L, -2, "type");
        
        push_address(L, lief_elf_section_flags(wrapper, i));
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
 * @return 返回1个结果: 二进制内容字符串
 * @description Lua调用: content = binary:section_content(index或name)
 *              支持两种参数类型:
 *              - 数字: 按索引获取 (从1开始)
 *              - 字符串: 按节名称获取 (如 ".text")
 */
static int lua_elf_section_content(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t index;
    
    /* 支持按索引或按名称获取节内容 */
    if (lua_type(L, 2) == LUA_TSTRING) {
        /* 按名称获取 */
        const char *name = lua_tostring(L, 2);
        int idx = lief_elf_get_section_index(wrapper, name);
        if (idx < 0) {
            lua_pushnil(L);
            return 1;
        }
        index = (size_t)idx;
    } else {
        /* 按索引获取 (Lua索引从1开始) */
        index = (size_t)luaL_checkinteger(L, 2) - 1;
    }
    
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
 * @return 返回1个结果: 布尔值表示是否成功
 * @description Lua调用: binary:set_section_content(index或name, content)
 *              支持两种参数类型:
 *              - 数字: 按索引设置 (从1开始)
 *              - 字符串: 按节名称设置 (如 ".text")
 */
static int lua_elf_set_section_content(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t index;
    
    /* 支持按索引或按名称设置节内容 */
    if (lua_type(L, 2) == LUA_TSTRING) {
        /* 按名称获取索引 */
        const char *name = lua_tostring(L, 2);
        int idx = lief_elf_get_section_index(wrapper, name);
        if (idx < 0) {
            lua_pushboolean(L, 0);
            return 1;
        }
        index = (size_t)idx;
    } else {
        /* 按索引 (Lua索引从1开始) */
        index = (size_t)luaL_checkinteger(L, 2) - 1;
    }
    
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
        
        push_address(L, lief_elf_segment_virtual_address(wrapper, i));
        lua_setfield(L, -2, "virtual_address");
        
        push_address(L, lief_elf_segment_virtual_size(wrapper, i));
        lua_setfield(L, -2, "virtual_size");
        
        push_address(L, lief_elf_segment_offset(wrapper, i));
        lua_setfield(L, -2, "offset");
        
        push_address(L, lief_elf_segment_file_size(wrapper, i));
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
        
        push_address(L, lief_elf_dynamic_symbol_value(wrapper, i));
        lua_setfield(L, -2, "value");
        
        push_address(L, lief_elf_dynamic_symbol_size(wrapper, i));
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
        
        push_address(L, lief_elf_dynamic_entry_tag(wrapper, i));
        lua_setfield(L, -2, "tag");
        
        push_address(L, lief_elf_dynamic_entry_value(wrapper, i));
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
    push_address(L, lief_elf_imagebase(wrapper));
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
    uint64_t va = check_address(L, 2);
    
    uint64_t offset;
    if (lief_elf_va_to_offset(wrapper, va, &offset) == 0) {
        push_address(L, offset);
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
    uint64_t offset = check_address(L, 2);
    
    uint64_t va;
    if (lief_elf_offset_to_va(wrapper, offset, &va) == 0) {
        push_address(L, va);
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
        
        push_address(L, lief_elf_relocation_address(wrapper, i));
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
    push_address(L, lief_elf_section_virtual_address(wrapper, idx));
    lua_setfield(L, -2, "virtual_address");
    push_address(L, lief_elf_section_size(wrapper, idx));
    lua_setfield(L, -2, "size");
    push_address(L, lief_elf_section_offset(wrapper, idx));
    lua_setfield(L, -2, "offset");
    lua_pushinteger(L, lief_elf_section_type(wrapper, idx));
    lua_setfield(L, -2, "type");
    push_address(L, lief_elf_section_flags(wrapper, idx));
    lua_setfield(L, -2, "flags");
    push_address(L, lief_elf_section_alignment(wrapper, idx));
    lua_setfield(L, -2, "alignment");
    push_address(L, lief_elf_section_entry_size(wrapper, idx));
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
    push_address(L, lief_elf_segment_virtual_address(wrapper, idx));
    lua_setfield(L, -2, "virtual_address");
    push_address(L, lief_elf_segment_physical_address(wrapper, idx));
    lua_setfield(L, -2, "physical_address");
    push_address(L, lief_elf_segment_virtual_size(wrapper, idx));
    lua_setfield(L, -2, "virtual_size");
    push_address(L, lief_elf_segment_file_size(wrapper, idx));
    lua_setfield(L, -2, "file_size");
    push_address(L, lief_elf_segment_offset(wrapper, idx));
    lua_setfield(L, -2, "offset");
    push_address(L, lief_elf_segment_alignment(wrapper, idx));
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
        
        push_address(L, lief_elf_symtab_symbol_value(wrapper, i));
        lua_setfield(L, -2, "value");
        
        push_address(L, lief_elf_symtab_symbol_size(wrapper, i));
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
    push_address(L, lief_elf_virtual_size(wrapper));
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
    push_address(L, lief_elf_eof_offset(wrapper));
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
 * @description Lua调用: section = binary:section_from_offset(offset)
 *              返回节信息表或nil
 */
static int lua_elf_section_from_offset(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t offset = (uint64_t)luaL_checkinteger(L, 2);
    
    int idx = lief_elf_section_from_offset(wrapper, offset);
    if (idx < 0) {
        lua_pushnil(L);
        return 1;
    }
    
    /* 返回节信息表 */
    lua_newtable(L);
    lua_pushstring(L, lief_elf_section_name(wrapper, idx));
    lua_setfield(L, -2, "name");
    push_address(L, lief_elf_section_virtual_address(wrapper, idx));
    lua_setfield(L, -2, "virtual_address");
    push_address(L, lief_elf_section_size(wrapper, idx));
    lua_setfield(L, -2, "size");
    push_address(L, lief_elf_section_offset(wrapper, idx));
    lua_setfield(L, -2, "offset");
    lua_pushinteger(L, lief_elf_section_type(wrapper, idx));
    lua_setfield(L, -2, "type");
    push_address(L, lief_elf_section_flags(wrapper, idx));
    lua_setfield(L, -2, "flags");
    lua_pushinteger(L, idx + 1);
    lua_setfield(L, -2, "index");
    
    return 1;
}

/**
 * @brief 从虚拟地址获取节
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: section = binary:section_from_va(va)
 *              返回节信息表或nil
 */
static int lua_elf_section_from_va(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t va = (uint64_t)luaL_checkinteger(L, 2);
    
    int idx = lief_elf_section_from_va(wrapper, va);
    if (idx < 0) {
        lua_pushnil(L);
        return 1;
    }
    
    /* 返回节信息表 */
    lua_newtable(L);
    lua_pushstring(L, lief_elf_section_name(wrapper, idx));
    lua_setfield(L, -2, "name");
    push_address(L, lief_elf_section_virtual_address(wrapper, idx));
    lua_setfield(L, -2, "virtual_address");
    push_address(L, lief_elf_section_size(wrapper, idx));
    lua_setfield(L, -2, "size");
    push_address(L, lief_elf_section_offset(wrapper, idx));
    lua_setfield(L, -2, "offset");
    lua_pushinteger(L, lief_elf_section_type(wrapper, idx));
    lua_setfield(L, -2, "type");
    push_address(L, lief_elf_section_flags(wrapper, idx));
    lua_setfield(L, -2, "flags");
    lua_pushinteger(L, idx + 1);
    lua_setfield(L, -2, "index");
    
    return 1;
}

/**
 * @brief 从偏移获取段
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: segment = binary:segment_from_offset(offset)
 *              返回段信息表或nil
 */
static int lua_elf_segment_from_offset(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t offset = (uint64_t)luaL_checkinteger(L, 2);
    
    int idx = lief_elf_segment_from_offset(wrapper, offset);
    if (idx < 0) {
        lua_pushnil(L);
        return 1;
    }
    
    /* 返回段信息表 */
    lua_newtable(L);
    lua_pushinteger(L, lief_elf_segment_type(wrapper, idx));
    lua_setfield(L, -2, "type");
    lua_pushinteger(L, lief_elf_segment_flags(wrapper, idx));
    lua_setfield(L, -2, "flags");
    push_address(L, lief_elf_segment_virtual_address(wrapper, idx));
    lua_setfield(L, -2, "virtual_address");
    push_address(L, lief_elf_segment_virtual_size(wrapper, idx));
    lua_setfield(L, -2, "virtual_size");
    push_address(L, lief_elf_segment_offset(wrapper, idx));
    lua_setfield(L, -2, "offset");
    push_address(L, lief_elf_segment_file_size(wrapper, idx));
    lua_setfield(L, -2, "file_size");
    lua_pushinteger(L, idx + 1);
    lua_setfield(L, -2, "index");
    
    return 1;
}

/**
 * @brief 从虚拟地址获取段
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: segment = binary:segment_from_va(va)
 *              返回段信息表或nil
 */
static int lua_elf_segment_from_va(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t va = (uint64_t)luaL_checkinteger(L, 2);
    
    int idx = lief_elf_segment_from_va(wrapper, va);
    if (idx < 0) {
        lua_pushnil(L);
        return 1;
    }
    
    /* 返回段信息表 */
    lua_newtable(L);
    lua_pushinteger(L, lief_elf_segment_type(wrapper, idx));
    lua_setfield(L, -2, "type");
    lua_pushinteger(L, lief_elf_segment_flags(wrapper, idx));
    lua_setfield(L, -2, "flags");
    push_address(L, lief_elf_segment_virtual_address(wrapper, idx));
    lua_setfield(L, -2, "virtual_address");
    push_address(L, lief_elf_segment_virtual_size(wrapper, idx));
    lua_setfield(L, -2, "virtual_size");
    push_address(L, lief_elf_segment_offset(wrapper, idx));
    lua_setfield(L, -2, "offset");
    push_address(L, lief_elf_segment_file_size(wrapper, idx));
    lua_setfield(L, -2, "file_size");
    lua_pushinteger(L, idx + 1);
    lua_setfield(L, -2, "index");
    
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

/* ========== 新增：反汇编功能 ========== */

/**
 * @brief 反汇编指定地址的代码
 * @param L Lua状态机指针
 * @return 返回1或2 (成功返回指令表，失败返回nil和错误信息)
 * @description Lua调用: instructions, err = binary:disassemble(address, size)
 */
static int lua_elf_disassemble(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t address = check_address(L, 2);
    size_t size = (size_t)luaL_checkinteger(L, 3);
    
    size_t count = 0;
    Disasm_Instruction *insts = lief_elf_disassemble(wrapper, address, size, &count);
    
    if (!insts || count == 0) {
        lua_pushnil(L);
        const char *err = lief_get_last_error();
        lua_pushstring(L, err ? err : "Disassemble failed (no instructions found)");
        return 2;
    }
    
    lua_newtable(L);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        push_address(L, insts[i].address);
        lua_setfield(L, -2, "address");
        
        lua_pushstring(L, insts[i].mnemonic);
        lua_setfield(L, -2, "mnemonic");
        
        lua_pushstring(L, insts[i].operands);
        lua_setfield(L, -2, "operands");
        
        lua_pushstring(L, insts[i].full_str);
        lua_setfield(L, -2, "text");
        
        lua_pushlstring(L, (const char*)insts[i].raw, insts[i].raw_size);
        lua_setfield(L, -2, "raw");
        
        lua_pushinteger(L, (lua_Integer)insts[i].raw_size);
        lua_setfield(L, -2, "size");
        
        lua_pushboolean(L, insts[i].is_call);
        lua_setfield(L, -2, "is_call");
        
        lua_pushboolean(L, insts[i].is_branch);
        lua_setfield(L, -2, "is_branch");
        
        lua_pushboolean(L, insts[i].is_return);
        lua_setfield(L, -2, "is_return");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    lief_elf_free_disasm(insts);
    return 1;
}

/**
 * @brief 反汇编缓冲区
 * @param L Lua状态机指针
 * @return 返回1或2 (成功返回指令表，失败返回nil和错误信息)
 * @description Lua调用: instructions, err = binary:disassemble_buffer(data, address)
 */
static int lua_elf_disassemble_buffer(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t size;
    const char *data = luaL_checklstring(L, 2, &size);
    uint64_t address = luaL_optnumber(L, 3, 0);
    
    size_t count = 0;
    Disasm_Instruction *insts = lief_elf_disassemble_buffer(wrapper, (const uint8_t*)data, size, address, &count);
    
    if (!insts || count == 0) {
        lua_pushnil(L);
        const char *err = lief_get_last_error();
        lua_pushstring(L, err ? err : "Disassemble buffer failed");
        return 2;
    }
    
    lua_newtable(L);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        push_address(L, insts[i].address);
        lua_setfield(L, -2, "address");
        
        lua_pushstring(L, insts[i].mnemonic);
        lua_setfield(L, -2, "mnemonic");
        
        lua_pushstring(L, insts[i].operands);
        lua_setfield(L, -2, "operands");
        
        lua_pushstring(L, insts[i].full_str);
        lua_setfield(L, -2, "text");
        
        lua_pushlstring(L, (const char*)insts[i].raw, insts[i].raw_size);
        lua_setfield(L, -2, "raw");
        
        lua_pushinteger(L, (lua_Integer)insts[i].raw_size);
        lua_setfield(L, -2, "size");
        
        lua_pushboolean(L, insts[i].is_call);
        lua_setfield(L, -2, "is_call");
        
        lua_pushboolean(L, insts[i].is_branch);
        lua_setfield(L, -2, "is_branch");
        
        lua_pushboolean(L, insts[i].is_return);
        lua_setfield(L, -2, "is_return");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    lief_elf_free_disasm(insts);
    return 1;
}

/**
 * @brief 反汇编符号
 * @param L Lua状态机指针
 * @return 返回1或2 (成功返回指令表，失败返回nil和错误信息)
 * @description Lua调用: instructions, err = binary:disassemble_symbol(name)
 */
static int lua_elf_disassemble_symbol(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    
    size_t count = 0;
    Disasm_Instruction *insts = lief_elf_disassemble_symbol(wrapper, name, &count);
    
    if (!insts || count == 0) {
        lua_pushnil(L);
        const char *err = lief_get_last_error();
        lua_pushstring(L, err ? err : "Disassemble symbol failed");
        return 2;
    }
    
    lua_newtable(L);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        push_address(L, insts[i].address);
        lua_setfield(L, -2, "address");
        
        lua_pushstring(L, insts[i].mnemonic);
        lua_setfield(L, -2, "mnemonic");
        
        lua_pushstring(L, insts[i].full_str);
        lua_setfield(L, -2, "text");
        
        lua_pushboolean(L, insts[i].is_call);
        lua_setfield(L, -2, "is_call");
        
        lua_pushboolean(L, insts[i].is_branch);
        lua_setfield(L, -2, "is_branch");
        
        lua_pushboolean(L, insts[i].is_return);
        lua_setfield(L, -2, "is_return");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    lief_elf_free_disasm(insts);
    return 1;
}

/**
 * @brief 汇编代码
 * @param L Lua状态机指针
 * @return 返回1或2 (成功返回字节码，失败返回nil和错误信息)
 * @description Lua调用: bytes, err = binary:assemble(address, asm_code)
 */
static int lua_elf_assemble(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t address = check_address(L, 2);
    const char *asm_code = luaL_checkstring(L, 3);
    
    size_t size = 0;
    uint8_t *bytes = lief_elf_assemble(wrapper, address, asm_code, &size);
    
    if (!bytes || size == 0) {
        lua_pushnil(L);
        const char *err = lief_get_last_error();
        lua_pushstring(L, err ? err : "Assemble failed");
        return 2;
    }
    
    lua_pushlstring(L, (const char*)bytes, size);
    free(bytes);
    return 1;
}

/**
 * @brief 汇编并打补丁
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: ok = binary:assemble_patch(address, asm_code)
 */
static int lua_elf_assemble_patch(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t address = check_address(L, 2);
    const char *asm_code = luaL_checkstring(L, 3);
    
    int result = lief_elf_assemble_patch(wrapper, address, asm_code);
    lua_pushboolean(L, result == 0);
    return 1;
}

/* ========== 新增：Header扩展 ========== */

/**
 * @brief 获取ELF类别(32/64位)
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: class = binary:elf_class()
 */
static int lua_elf_class(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushinteger(L, lief_elf_get_class(wrapper));
    return 1;
}

/**
 * @brief 获取字节序
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: endian = binary:endianness()
 */
static int lua_elf_endianness(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushinteger(L, lief_elf_get_endianness(wrapper));
    return 1;
}

/**
 * @brief 获取程序头偏移
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: offset = binary:program_header_offset()
 */
static int lua_elf_program_header_offset(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    push_address(L, lief_elf_get_program_header_offset(wrapper));
    return 1;
}

/**
 * @brief 获取节头偏移
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: offset = binary:section_header_offset()
 */
static int lua_elf_section_header_offset(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    push_address(L, lief_elf_get_section_header_offset(wrapper));
    return 1;
}

/**
 * @brief 获取页面大小
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: size = binary:page_size()
 */
static int lua_elf_page_size(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    push_address(L, lief_elf_page_size(wrapper));
    return 1;
}

/* ========== 新增：库依赖列表 ========== */

/**
 * @brief 获取库依赖列表
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: libs = binary:libraries()
 */
static int lua_elf_libraries(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    size_t count = lief_elf_libraries_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        const char *name = lief_elf_library_name(wrapper, i);
        lua_pushstring(L, name ? name : "");
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    return 1;
}

/* ========== 新增：导出/导入函数 ========== */

/**
 * @brief 获取导出函数列表
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: funcs = binary:exported_functions()
 */
static int lua_elf_exported_functions(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    size_t count = lief_elf_exported_functions_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        const char *name = lief_elf_exported_function_name(wrapper, i);
        lua_pushstring(L, name ? name : "");
        lua_setfield(L, -2, "name");
        
        push_address(L, lief_elf_exported_function_address(wrapper, i));
        lua_setfield(L, -2, "address");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    return 1;
}

/**
 * @brief 获取导入函数列表
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: funcs = binary:imported_functions()
 */
static int lua_elf_imported_functions(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    size_t count = lief_elf_imported_functions_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        const char *name = lief_elf_imported_function_name(wrapper, i);
        lua_pushstring(L, name ? name : "");
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    return 1;
}

/* ========== 新增：构造函数 ========== */

/**
 * @brief 获取构造函数列表
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: ctors = binary:ctor_functions()
 */
static int lua_elf_ctor_functions(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    size_t count = lief_elf_ctor_functions_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        push_address(L, lief_elf_ctor_function_address(wrapper, i));
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    return 1;
}

/* ========== 新增：Note操作 ========== */

/**
 * @brief 获取Note列表
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: notes = binary:notes()
 */
static int lua_elf_notes(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    size_t count = lief_elf_notes_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        const char *name = lief_elf_note_name(wrapper, i);
        lua_pushstring(L, name ? name : "");
        lua_setfield(L, -2, "name");
        
        lua_pushinteger(L, lief_elf_note_type(wrapper, i));
        lua_setfield(L, -2, "type");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    
    return 1;
}

/* ========== 新增：SONAME/RUNPATH ========== */

/**
 * @brief 获取SONAME
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: name = binary:soname()
 */
static int lua_elf_soname(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *soname = lief_elf_get_soname(wrapper);
    if (soname) {
        lua_pushstring(L, soname);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 设置SONAME
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: ok = binary:set_soname(name)
 */
static int lua_elf_set_soname(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *soname = luaL_checkstring(L, 2);
    int result = lief_elf_set_soname(wrapper, soname);
    lua_pushboolean(L, result == 0);
    return 1;
}

/**
 * @brief 获取RUNPATH
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: path = binary:runpath()
 */
static int lua_elf_runpath(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *runpath = lief_elf_get_runpath(wrapper);
    if (runpath) {
        lua_pushstring(L, runpath);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 设置RUNPATH
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: ok = binary:set_runpath(path)
 */
static int lua_elf_set_runpath(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *runpath = luaL_checkstring(L, 2);
    int result = lief_elf_set_runpath(wrapper, runpath);
    lua_pushboolean(L, result == 0);
    return 1;
}

/* ========== 新增：数量统计 ========== */

/**
 * @brief 获取各节数量统计
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: counts = binary:counts()
 */
static int lua_elf_counts(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    lua_pushinteger(L, (lua_Integer)lief_elf_sections_count(wrapper));
    lua_setfield(L, -2, "sections");
    
    lua_pushinteger(L, (lua_Integer)lief_elf_segments_count(wrapper));
    lua_setfield(L, -2, "segments");
    
    lua_pushinteger(L, (lua_Integer)lief_elf_dynamic_symbols_count(wrapper));
    lua_setfield(L, -2, "dynamic_symbols");
    
    lua_pushinteger(L, (lua_Integer)lief_elf_symtab_symbols_count(wrapper));
    lua_setfield(L, -2, "symtab_symbols");
    
    lua_pushinteger(L, (lua_Integer)lief_elf_dynamic_entries_count(wrapper));
    lua_setfield(L, -2, "dynamic_entries");
    
    lua_pushinteger(L, (lua_Integer)lief_elf_relocations_count(wrapper));
    lua_setfield(L, -2, "relocations");
    
    lua_pushinteger(L, (lua_Integer)lief_elf_libraries_count(wrapper));
    lua_setfield(L, -2, "libraries");
    
    lua_pushinteger(L, (lua_Integer)lief_elf_notes_count(wrapper));
    lua_setfield(L, -2, "notes");
    
    lua_pushinteger(L, (lua_Integer)lief_elf_exported_functions_count(wrapper));
    lua_setfield(L, -2, "exported_functions");
    
    lua_pushinteger(L, (lua_Integer)lief_elf_imported_functions_count(wrapper));
    lua_setfield(L, -2, "imported_functions");
    
    lua_pushinteger(L, (lua_Integer)lief_elf_ctor_functions_count(wrapper));
    lua_setfield(L, -2, "ctor_functions");
    
    return 1;
}

/* ========== 新增：辅助检查 ========== */

/**
 * @brief 检查是否有调试信息
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_debug_info()
 */
static int lua_elf_has_debug_info(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushboolean(L, lief_elf_has_debug_info(wrapper));
    return 1;
}

/**
 * @brief 检查是否有GNU Hash
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_gnu_hash()
 */
static int lua_elf_has_gnu_hash(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushboolean(L, lief_elf_has_gnu_hash(wrapper));
    return 1;
}

/**
 * @brief 地址格式化辅助函数
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: str = liefx.format_address(addr)
 */
static int lua_format_address(lua_State *L) {
    uint64_t addr = check_address(L, 1);
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "0x%llX", (unsigned long long)addr);
    lua_pushstring(L, buffer);
    return 1;
}

/* ========== 新增：符号查找 ========== */

/**
 * @brief 检查是否有指定动态符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_dynamic_symbol(name)
 */
static int lua_elf_has_dynamic_symbol(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    lua_pushboolean(L, lief_elf_has_dynamic_symbol(wrapper, name));
    return 1;
}

/**
 * @brief 检查是否有指定symtab符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_symtab_symbol(name)
 */
static int lua_elf_has_symtab_symbol(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    lua_pushboolean(L, lief_elf_has_symtab_symbol(wrapper, name));
    return 1;
}

/**
 * @brief 按名称获取动态符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: sym = binary:get_dynamic_symbol(name)
 */
static int lua_elf_get_dynamic_symbol_by_name(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    
    uint64_t value, size;
    uint32_t type, binding;
    
    if (lief_elf_get_dynamic_symbol(wrapper, name, &value, &size, &type, &binding) == 0) {
        lua_newtable(L);
        lua_pushstring(L, name);
        lua_setfield(L, -2, "name");
        push_address(L, value);
        lua_setfield(L, -2, "value");
        push_address(L, size);
        lua_setfield(L, -2, "size");
        lua_pushinteger(L, type);
        lua_setfield(L, -2, "type");
        lua_pushinteger(L, binding);
        lua_setfield(L, -2, "binding");
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 按名称获取symtab符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: sym = binary:get_symtab_symbol(name)
 */
static int lua_elf_get_symtab_symbol_by_name(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    
    uint64_t value, size;
    uint32_t type, binding;
    
    if (lief_elf_get_symtab_symbol(wrapper, name, &value, &size, &type, &binding) == 0) {
        lua_newtable(L);
        lua_pushstring(L, name);
        lua_setfield(L, -2, "name");
        push_address(L, value);
        lua_setfield(L, -2, "value");
        push_address(L, size);
        lua_setfield(L, -2, "size");
        lua_pushinteger(L, type);
        lua_setfield(L, -2, "type");
        lua_pushinteger(L, binding);
        lua_setfield(L, -2, "binding");
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* ========== 新增：重定位查找 ========== */

/**
 * @brief 按地址获取重定位
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: reloc = binary:get_relocation_by_address(addr)
 */
static int lua_elf_get_relocation_by_address(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t address = check_address(L, 2);
    
    uint32_t type;
    int64_t addend;
    
    if (lief_elf_get_relocation_by_address(wrapper, address, &type, &addend) == 0) {
        lua_newtable(L);
        push_address(L, address);
        lua_setfield(L, -2, "address");
        lua_pushinteger(L, type);
        lua_setfield(L, -2, "type");
        lua_pushinteger(L, (lua_Integer)addend);
        lua_setfield(L, -2, "addend");
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 按符号名获取重定位
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: reloc = binary:get_relocation_by_symbol(name)
 */
static int lua_elf_get_relocation_by_symbol(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    
    uint64_t address;
    uint32_t type;
    int64_t addend;
    
    if (lief_elf_get_relocation_by_symbol(wrapper, name, &address, &type, &addend) == 0) {
        lua_newtable(L);
        push_address(L, address);
        lua_setfield(L, -2, "address");
        lua_pushinteger(L, type);
        lua_setfield(L, -2, "type");
        lua_pushinteger(L, (lua_Integer)addend);
        lua_setfield(L, -2, "addend");
        lua_pushstring(L, name);
        lua_setfield(L, -2, "symbol");
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* ========== 新增：GNU Hash检查 ========== */

/**
 * @brief 使用GNU Hash检查符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: maybe = binary:gnu_hash_check(name)
 */
static int lua_elf_gnu_hash_check(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    lua_pushboolean(L, lief_elf_gnu_hash_check(wrapper, name));
    return 1;
}

/* ========== 新增：字符串提取 ========== */

/**
 * @brief 从.rodata提取字符串
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: strings = binary:strings(min_size)
 */
static int lua_elf_strings(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    size_t min_size = (size_t)luaL_optinteger(L, 2, 5);
    
    size_t count = 0;
    Elf_String *strings = lief_elf_strings(wrapper, min_size, &count);
    
    lua_newtable(L);
    if (strings && count > 0) {
        for (size_t i = 0; i < count; i++) {
            lua_pushstring(L, strings[i].str ? strings[i].str : "");
            lua_rawseti(L, -2, (int)(i + 1));
        }
        lief_elf_free_strings(strings, count);
    }
    return 1;
}

/* ========== 新增：虚拟地址辅助 ========== */

/**
 * @brief 获取下一个可用虚拟地址
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: va = binary:next_virtual_address()
 */
static int lua_elf_next_virtual_address(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    push_address(L, lief_elf_next_virtual_address(wrapper));
    return 1;
}

/**
 * @brief 获取节表最后偏移
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: offset = binary:last_offset_section()
 */
static int lua_elf_last_offset_section(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    push_address(L, lief_elf_last_offset_section(wrapper));
    return 1;
}

/**
 * @brief 获取段表最后偏移
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: offset = binary:last_offset_segment()
 */
static int lua_elf_last_offset_segment(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    push_address(L, lief_elf_last_offset_segment(wrapper));
    return 1;
}

/* ========== 新增：导入/导出符号 ========== */

/**
 * @brief 获取导入符号列表
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: syms = binary:imported_symbols()
 */
static int lua_elf_imported_symbols(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    size_t count = lief_elf_imported_symbols_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        const char *name = lief_elf_imported_symbol_name(wrapper, i);
        lua_pushstring(L, name ? name : "");
        lua_rawseti(L, -2, (int)(i + 1));
    }
    return 1;
}

/**
 * @brief 获取导出符号列表
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: syms = binary:exported_symbols()
 */
static int lua_elf_exported_symbols(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    size_t count = lief_elf_exported_symbols_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        const char *name = lief_elf_exported_symbol_name(wrapper, i);
        lua_pushstring(L, name ? name : "");
        lua_setfield(L, -2, "name");
        
        push_address(L, lief_elf_exported_symbol_value(wrapper, i));
        lua_setfield(L, -2, "value");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    return 1;
}

/**
 * @brief 移除所有同名符号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: ok = binary:remove_symbol(name)
 */
static int lua_elf_remove_symbol(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    int result = lief_elf_remove_symbol(wrapper, name);
    lua_pushboolean(L, result == 0);
    return 1;
}

/* ========== 新增：安全检查 ========== */

/**
 * @brief 获取RELRO保护类型
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: type = binary:relro_type() -- 0=无, 1=部分, 2=完全
 */
static int lua_elf_relro_type(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushinteger(L, lief_elf_relro_type(wrapper));
    return 1;
}

/**
 * @brief 检查是否有栈金丝雀保护
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_stack_canary()
 */
static int lua_elf_has_stack_canary(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushboolean(L, lief_elf_has_stack_canary(wrapper));
    return 1;
}

/**
 * @brief 检查是否有FORTIFY保护
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_fortify()
 */
static int lua_elf_has_fortify(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushboolean(L, lief_elf_has_fortify(wrapper));
    return 1;
}

/**
 * @brief 检查是否有RPATH
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_rpath()
 */
static int lua_elf_has_rpath(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushboolean(L, lief_elf_has_rpath(wrapper));
    return 1;
}

/**
 * @brief 获取安全信息汇总
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: info = binary:security_info()
 */
static int lua_elf_security_info(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    
    /* PIE */
    lua_pushboolean(L, lief_elf_is_pie(wrapper));
    lua_setfield(L, -2, "pie");
    
    /* NX */
    lua_pushboolean(L, lief_elf_has_nx(wrapper));
    lua_setfield(L, -2, "nx");
    
    /* RELRO */
    int relro = lief_elf_relro_type(wrapper);
    lua_pushstring(L, relro == 2 ? "Full" : (relro == 1 ? "Partial" : "None"));
    lua_setfield(L, -2, "relro");
    
    /* Stack Canary */
    lua_pushboolean(L, lief_elf_has_stack_canary(wrapper));
    lua_setfield(L, -2, "canary");
    
    /* FORTIFY */
    lua_pushboolean(L, lief_elf_has_fortify(wrapper));
    lua_setfield(L, -2, "fortify");
    
    /* RPATH (安全风险) */
    lua_pushboolean(L, lief_elf_has_rpath(wrapper));
    lua_setfield(L, -2, "rpath");
    
    return 1;
}

/**
 * @brief 使用自定义配置写入
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: ok = binary:write_config(path, {rebuild_hash=true, rebuild_symtab=true})
 */
static int lua_elf_write_with_config(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *filepath = luaL_checkstring(L, 2);
    
    int rebuild_hash = 1;
    int rebuild_symtab = 1;
    
    if (lua_istable(L, 3)) {
        lua_getfield(L, 3, "rebuild_hash");
        if (!lua_isnil(L, -1)) {
            rebuild_hash = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);
        
        lua_getfield(L, 3, "rebuild_symtab");
        if (!lua_isnil(L, -1)) {
            rebuild_symtab = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);
    }
    
    int result = lief_elf_write_with_config(wrapper, filepath, rebuild_hash, rebuild_symtab);
    lua_pushboolean(L, result == 0);
    return 1;
}

/* ========== 新增：Android特定信息 ========== */

/**
 * @brief 获取Android SDK版本
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: ver = binary:android_sdk_version()
 */
static int lua_elf_android_sdk_version(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint32_t version = lief_elf_android_sdk_version(wrapper);
    if (version == 0) {
        lua_pushnil(L);
    } else {
        lua_pushinteger(L, version);
    }
    return 1;
}

/**
 * @brief 获取Android NDK版本
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: ver = binary:android_ndk_version()
 */
static int lua_elf_android_ndk_version(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *version = lief_elf_android_ndk_version(wrapper);
    if (version) {
        lua_pushstring(L, version);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 获取Android NDK构建号
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: build = binary:android_ndk_build_number()
 */
static int lua_elf_android_ndk_build_number(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *build = lief_elf_android_ndk_build_number(wrapper);
    if (build) {
        lua_pushstring(L, build);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/**
 * @brief 获取Android完整信息
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: info = binary:android_info()
 */
static int lua_elf_android_info(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    uint32_t sdk = lief_elf_android_sdk_version(wrapper);
    const char *ndk_ver = lief_elf_android_ndk_version(wrapper);
    const char *ndk_build = lief_elf_android_ndk_build_number(wrapper);
    
    if (sdk == 0 && !ndk_ver && !ndk_build) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_newtable(L);
    
    if (sdk > 0) {
        lua_pushinteger(L, sdk);
        lua_setfield(L, -2, "sdk_version");
    }
    
    if (ndk_ver && strlen(ndk_ver) > 0) {
        lua_pushstring(L, ndk_ver);
        lua_setfield(L, -2, "ndk_version");
    }
    
    if (ndk_build && strlen(ndk_build) > 0) {
        lua_pushstring(L, ndk_build);
        lua_setfield(L, -2, "ndk_build_number");
    }
    
    return 1;
}

/* ========== 新增：ABI信息 ========== */

/**
 * @brief 获取ABI类型
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: abi = binary:abi_type()
 */
static int lua_elf_abi_type(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    int abi = lief_elf_abi_type(wrapper);
    if (abi < 0) {
        lua_pushnil(L);
    } else {
        /* 返回ABI名称字符串 */
        const char *abi_names[] = {"Linux", "GNU", "Solaris", "FreeBSD", "NetBSD", "Syllable", "NaCl"};
        if (abi >= 0 && abi < 7) {
            lua_pushstring(L, abi_names[abi]);
        } else {
            lua_pushstring(L, "Unknown");
        }
    }
    return 1;
}

/**
 * @brief 获取ABI版本
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: ver = binary:abi_version() -- 返回{major, minor, patch}
 */
static int lua_elf_abi_version(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    uint32_t major, minor, patch;
    if (lief_elf_abi_version(wrapper, &major, &minor, &patch) == 0) {
        lua_newtable(L);
        lua_pushinteger(L, major);
        lua_setfield(L, -2, "major");
        lua_pushinteger(L, minor);
        lua_setfield(L, -2, "minor");
        lua_pushinteger(L, patch);
        lua_setfield(L, -2, "patch");
        
        /* 也提供字符串形式 */
        lua_pushfstring(L, "%d.%d.%d", major, minor, patch);
        lua_setfield(L, -2, "string");
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* ========== 新增：Hash表详细信息 ========== */

/**
 * @brief 获取GNU Hash详细信息
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: hash = binary:gnu_hash_info()
 */
static int lua_elf_gnu_hash_info(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    if (!lief_elf_has_gnu_hash(wrapper)) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_newtable(L);
    
    lua_pushinteger(L, lief_elf_gnu_hash_symbol_index(wrapper));
    lua_setfield(L, -2, "symbol_index");
    
    lua_pushinteger(L, lief_elf_gnu_hash_nb_buckets(wrapper));
    lua_setfield(L, -2, "nb_buckets");
    
    lua_pushinteger(L, lief_elf_gnu_hash_shift2(wrapper));
    lua_setfield(L, -2, "shift2");
    
    lua_pushinteger(L, lief_elf_gnu_hash_maskwords(wrapper));
    lua_setfield(L, -2, "maskwords");
    
    return 1;
}

/**
 * @brief 获取SYSV Hash详细信息
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: hash = binary:sysv_hash_info()
 */
static int lua_elf_sysv_hash_info(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    uint32_t nbucket = lief_elf_sysv_hash_nbucket(wrapper);
    uint32_t nchain = lief_elf_sysv_hash_nchain(wrapper);
    
    if (nbucket == 0 && nchain == 0) {
        lua_pushnil(L);
        return 1;
    }
    
    lua_newtable(L);
    
    lua_pushinteger(L, nbucket);
    lua_setfield(L, -2, "nbucket");
    
    lua_pushinteger(L, nchain);
    lua_setfield(L, -2, "nchain");
    
    return 1;
}

/* ========== 新增：符号版本信息 ========== */

/**
 * @brief 获取符号版本需求列表
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: reqs = binary:symbol_version_requirements()
 */
static int lua_elf_symbol_version_requirements(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    size_t count = lief_elf_symbol_version_requirements_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        const char *name = lief_elf_symbol_version_requirement_name(wrapper, i);
        lua_pushstring(L, name ? name : "");
        lua_rawseti(L, -2, (int)(i + 1));
    }
    return 1;
}

/**
 * @brief 获取符号版本定义数量
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: count = binary:symbol_version_definitions_count()
 */
static int lua_elf_symbol_version_definitions_count(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    lua_pushinteger(L, (lua_Integer)lief_elf_symbol_version_definitions_count(wrapper));
    return 1;
}

/**
 * @brief 删除符号版本需求
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: ok = binary:remove_version_requirement(libname)
 */
static int lua_elf_remove_version_requirement(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *libname = luaL_checkstring(L, 2);
    int result = lief_elf_remove_version_requirement(wrapper, libname);
    lua_pushboolean(L, result == 0);
    return 1;
}

/* ========== 新增：函数辅助功能 ========== */

/**
 * @brief 获取析构函数列表
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: dtors = binary:dtor_functions()
 */
static int lua_elf_dtor_functions(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    size_t count = lief_elf_dtor_functions_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        push_address(L, lief_elf_dtor_function_address(wrapper, i));
        lua_rawseti(L, -2, (int)(i + 1));
    }
    return 1;
}

/**
 * @brief 获取所有函数列表
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: funcs = binary:functions()
 */
static int lua_elf_functions(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    
    lua_newtable(L);
    size_t count = lief_elf_functions_count(wrapper);
    for (size_t i = 0; i < count; i++) {
        lua_newtable(L);
        
        const char *name = lief_elf_function_name(wrapper, i);
        lua_pushstring(L, name ? name : "");
        lua_setfield(L, -2, "name");
        
        push_address(L, lief_elf_function_address(wrapper, i));
        lua_setfield(L, -2, "address");
        
        lua_rawseti(L, -2, (int)(i + 1));
    }
    return 1;
}

/**
 * @brief 通过名称获取函数地址
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: addr = binary:get_function_address(name)
 */
static int lua_elf_get_function_address(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    
    uint64_t address;
    if (lief_elf_get_function_address(wrapper, name, &address) == 0) {
        push_address(L, address);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/* ========== 新增：符号索引查找 ========== */

/**
 * @brief 获取动态符号索引
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: idx = binary:dynsym_idx(name)
 */
static int lua_elf_dynsym_idx(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    int64_t idx = lief_elf_dynsym_idx(wrapper, name);
    lua_pushinteger(L, (lua_Integer)idx);
    return 1;
}

/**
 * @brief 获取symtab符号索引
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: idx = binary:symtab_idx(name)
 */
static int lua_elf_symtab_idx(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    const char *name = luaL_checkstring(L, 2);
    int64_t idx = lief_elf_symtab_idx(wrapper, name);
    lua_pushinteger(L, (lua_Integer)idx);
    return 1;
}

/* ========== 新增：其他辅助功能 ========== */

/**
 * @brief 检查是否有指定偏移的节
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_section_with_offset(offset)
 */
static int lua_elf_has_section_with_offset(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t offset = check_address(L, 2);
    lua_pushboolean(L, lief_elf_has_section_with_offset(wrapper, offset));
    return 1;
}

/**
 * @brief 检查是否有指定虚拟地址的节
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: has = binary:has_section_with_va(va)
 */
static int lua_elf_has_section_with_va(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t va = check_address(L, 2);
    lua_pushboolean(L, lief_elf_has_section_with_va(wrapper, va));
    return 1;
}

/**
 * @brief 获取.text节索引
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: idx = binary:text_section_index()
 */
static int lua_elf_text_section_index(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    int idx = lief_elf_text_section_index(wrapper);
    lua_pushinteger(L, idx);
    return 1;
}

/**
 * @brief 获取.dynamic节索引
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: idx = binary:dynamic_section_index()
 */
static int lua_elf_dynamic_section_index(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    int idx = lief_elf_dynamic_section_index(wrapper);
    lua_pushinteger(L, idx);
    return 1;
}

/**
 * @brief 获取重定位后的动态数组
 * @param L Lua状态机指针
 * @return 返回1
 * @description Lua调用: arr = binary:get_relocated_dynamic_array(tag)
 */
static int lua_elf_get_relocated_dynamic_array(lua_State *L) {
    Elf_Binary_Wrapper *wrapper = check_elf_binary(L, 1);
    uint64_t tag = (uint64_t)luaL_checkinteger(L, 2);
    
    uint64_t *values = NULL;
    size_t count = 0;
    
    if (lief_elf_get_relocated_dynamic_array(wrapper, tag, &values, &count) == 0) {
        lua_newtable(L);
        for (size_t i = 0; i < count; i++) {
            push_address(L, values[i]);
            lua_rawseti(L, -2, (int)(i + 1));
        }
        lief_elf_free_dynamic_array(values);
    } else {
        lua_newtable(L);
    }
    return 1;
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
    {"elf_class",            lua_elf_class},
    {"endianness",           lua_elf_endianness},
    {"program_header_offset", lua_elf_program_header_offset},
    {"section_header_offset", lua_elf_section_header_offset},
    {"page_size",            lua_elf_page_size},
    
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
    {"libraries",            lua_elf_libraries},
    
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
    
    /* 反汇编/汇编 */
    {"disassemble",          lua_elf_disassemble},
    {"disassemble_buffer",   lua_elf_disassemble_buffer},
    {"disassemble_symbol",   lua_elf_disassemble_symbol},
    {"assemble",             lua_elf_assemble},
    {"assemble_patch",       lua_elf_assemble_patch},
    
    /* 导出/导入函数 */
    {"exported_functions",   lua_elf_exported_functions},
    {"imported_functions",   lua_elf_imported_functions},
    {"ctor_functions",       lua_elf_ctor_functions},
    
    /* Note */
    {"notes",                lua_elf_notes},
    
    /* SONAME/RUNPATH */
    {"soname",               lua_elf_soname},
    {"set_soname",           lua_elf_set_soname},
    {"runpath",              lua_elf_runpath},
    {"set_runpath",          lua_elf_set_runpath},
    
    /* 数量统计 */
    {"counts",               lua_elf_counts},
    
    /* 符号查找 */
    {"has_dynamic_symbol",   lua_elf_has_dynamic_symbol},
    {"has_symtab_symbol",    lua_elf_has_symtab_symbol},
    {"get_dynamic_symbol",   lua_elf_get_dynamic_symbol_by_name},
    {"get_symtab_symbol",    lua_elf_get_symtab_symbol_by_name},
    {"remove_symbol",        lua_elf_remove_symbol},
    {"imported_symbols",     lua_elf_imported_symbols},
    {"exported_symbols",     lua_elf_exported_symbols},
    
    /* 重定位查找 */
    {"get_relocation_by_address", lua_elf_get_relocation_by_address},
    {"get_relocation_by_symbol",  lua_elf_get_relocation_by_symbol},
    
    /* GNU Hash */
    {"gnu_hash_check",       lua_elf_gnu_hash_check},
    
    /* 字符串提取 */
    {"strings",              lua_elf_strings},
    
    /* 虚拟地址辅助 */
    {"next_virtual_address", lua_elf_next_virtual_address},
    {"last_offset_section",  lua_elf_last_offset_section},
    {"last_offset_segment",  lua_elf_last_offset_segment},
    
    /* 安全检查 */
    {"relro_type",           lua_elf_relro_type},
    {"has_stack_canary",     lua_elf_has_stack_canary},
    {"has_fortify",          lua_elf_has_fortify},
    {"has_rpath",            lua_elf_has_rpath},
    {"security_info",        lua_elf_security_info},
    
    /* 自定义写入 */
    {"write_config",         lua_elf_write_with_config},
    
    /* Android特定信息 */
    {"android_sdk_version",  lua_elf_android_sdk_version},
    {"android_ndk_version",  lua_elf_android_ndk_version},
    {"android_ndk_build_number", lua_elf_android_ndk_build_number},
    {"android_info",         lua_elf_android_info},
    
    /* ABI信息 */
    {"abi_type",             lua_elf_abi_type},
    {"abi_version",          lua_elf_abi_version},
    
    /* Hash表详细信息 */
    {"gnu_hash_info",        lua_elf_gnu_hash_info},
    {"sysv_hash_info",       lua_elf_sysv_hash_info},
    
    /* 符号版本信息 */
    {"symbol_version_requirements", lua_elf_symbol_version_requirements},
    {"symbol_version_definitions_count", lua_elf_symbol_version_definitions_count},
    {"remove_version_requirement", lua_elf_remove_version_requirement},
    
    /* 函数辅助功能 */
    {"dtor_functions",       lua_elf_dtor_functions},
    {"functions",            lua_elf_functions},
    {"get_function_address", lua_elf_get_function_address},
    
    /* 符号索引查找 */
    {"dynsym_idx",           lua_elf_dynsym_idx},
    {"symtab_idx",           lua_elf_symtab_idx},
    
    /* 其他辅助功能 */
    {"has_section_with_offset", lua_elf_has_section_with_offset},
    {"has_section_with_va",  lua_elf_has_section_with_va},
    {"text_section_index",   lua_elf_text_section_index},
    {"dynamic_section_index", lua_elf_dynamic_section_index},
    {"get_relocated_dynamic_array", lua_elf_get_relocated_dynamic_array},
    
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
    {"has_debug_info",       lua_elf_has_debug_info},
    {"has_gnu_hash",         lua_elf_has_gnu_hash},
    
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
    {"format_address",    lua_format_address},
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
