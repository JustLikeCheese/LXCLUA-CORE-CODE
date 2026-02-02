#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <android/log.h>
#include <sys/stat.h>
#include <time.h>

// 日志标签
#define LOG_TAG "LuaMemHack"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// 内存区域结构体
typedef struct {
    uintptr_t start;
    uintptr_t end;
    char permissions[5];
    char name[256];
} mem_region_t;

// LuaS魔数定义
static const uint8_t MY_LUA_SIGNATURE[] = {0x1B, 'L', 'u', 'a'};
static const uint8_t MY_LUAJIT_SIGNATURE[] = {0x1B, 'J', 0x00, 0x19};

/**
 * 读取进程内存映射
 * @param regions 输出参数，存储内存区域列表
 * @param count 输出参数，内存区域数量
 * @return 0表示成功，-1表示失败
 */
static int read_memory_maps(mem_region_t **regions, size_t *count) {
    int fd = open("/proc/self/maps", O_RDONLY);
    if (fd < 0) {
        LOGE("Failed to open /proc/self/maps: %s", strerror(errno));
        return -1;
    }

    // 预分配内存
    size_t capacity = 128;
    mem_region_t *temp_regions = malloc(capacity * sizeof(mem_region_t));
    if (!temp_regions) {
        close(fd);
        LOGE("Failed to allocate memory for regions");
        return -1;
    }

    size_t current_count = 0;
    char buffer[1024];
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        char *line = buffer;
        char *next_line;

        while ((next_line = strchr(line, '\n')) != NULL) {
            *next_line = '\0';
            
            // 解析内存映射行
            mem_region_t region;
            if (sscanf(line, "%lx-%lx %4s %*s %*s %*s %255s",
                      &region.start, &region.end, region.permissions, region.name) >= 3) {
                // 检查权限是否可读
                if (strchr(region.permissions, 'r') != NULL) {
                    // 过滤掉系统目录
                    bool is_system = false;
                    const char *system_dirs[] = {
                        "/system/",
                        "/vendor/",
                        "/lib/",
                        "/lib64/",
                        "/odm/",
                        "/product/",
                        "/apex/",
                        "/dev/",
                        "/proc/",
                        NULL
                    };
                    
                    // 检查是否是系统目录
                    for (int i = 0; system_dirs[i] != NULL; i++) {
                        if (strncmp(region.name, system_dirs[i], strlen(system_dirs[i])) == 0) {
                            is_system = true;
                            break;
                        }
                    }
                    
                    // 只保留非系统目录和匿名映射（空名称）
                    if (!is_system || region.name[0] == '\0') {
                        // 扩展内存（如果需要）
                        if (current_count >= capacity) {
                            capacity *= 2;
                            mem_region_t *new_regions = realloc(temp_regions, capacity * sizeof(mem_region_t));
                            if (!new_regions) {
                                free(temp_regions);
                                close(fd);
                                LOGE("Failed to reallocate memory for regions");
                                return -1;
                            }
                            temp_regions = new_regions;
                        }
                        temp_regions[current_count++] = region;
                    }
                }
            }
            line = next_line + 1;
        }
    }

    close(fd);

    if (bytes_read < 0) {
        free(temp_regions);
        LOGE("Failed to read /proc/self/maps: %s", strerror(errno));
        return -1;
    }

    // 调整内存大小
    mem_region_t *final_regions = realloc(temp_regions, current_count * sizeof(mem_region_t));
    if (!final_regions) {
        free(temp_regions);
        LOGE("Failed to reallocate final memory for regions");
        return -1;
    }

    *regions = final_regions;
    *count = current_count;
    return 0;
}

/**
 * 创建目录（递归）
 * @param path 目录路径
 * @return 0表示成功，-1表示失败
 */
static int mkdir_recursive(const char *path, mode_t mode) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);

    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
        return -1;
    }

    return 0;
}

/**
 * 获取当前日期字符串（YYYY-MM-DD格式）
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @return 0表示成功，-1表示失败
 */
static int get_current_date(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    if (!tm_info) {
        return -1;
    }
    return strftime(buffer, size, "%Y-%m-%d", tm_info);
}

/**
 * 获取当前时间字符串（HH-MM-SS格式）
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @return 0表示成功，-1表示失败
 */
static int get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    if (!tm_info) {
        return -1;
    }
    return strftime(buffer, size, "%H-%M-%S", tm_info);
}

/**
 * 保存匹配的数据到单独的文件
 * @param ptr 数据指针
 * @param size 数据大小
 * @param address 内存地址
 * @param type_name 数据类型名称
 * @return 0表示成功，-1表示失败
 */
static int save_match_data(const uint8_t *ptr, size_t size, uintptr_t address, const char *type_name) {
    // 创建数据目录
    const char *data_dir = "/sdcard/XCLUA/Mem/Data";
    if (mkdir_recursive(data_dir, 0755) != 0) {
        LOGE("Failed to create data directory: %s", strerror(errno));
        return -1;
    }
    
    // 生成文件名：/sdcard/XCLUA/Mem/Data/时间戳.lua
    char file_path[256];
    time_t now = time(NULL);
    snprintf(file_path, sizeof(file_path), "%s/%ld.lua", data_dir, now);
    
    // 打开文件
    FILE *data_file = fopen(file_path, "w");
    if (!data_file) {
        LOGE("Failed to open data file: %s", strerror(errno));
        return -1;
    }
    
    // 写入文件头
    fprintf(data_file, "-- Lua内存扫描匹配结果\n");
    fprintf(data_file, "-- 时间: %ld\n", now);
    fprintf(data_file, "-- 地址: 0x%lx\n", address);
    fprintf(data_file, "-- 类型: %s\n", type_name);
    fprintf(data_file, "-- 大小: %zu bytes\n\n", size);
    
    // 写入原始数据（十六进制格式）
    fprintf(data_file, "local data = '\\x");
    for (size_t j = 0; j < size; j++) {
        fprintf(data_file, "%02x", ptr[j]);
    }
    fprintf(data_file, "'\n\n");
    
    // 写入使用示例
    fprintf(data_file, "-- 使用示例:\n");
    fprintf(data_file, "-- loadstring(data)()\n");
    fprintf(data_file, "-- 或\n");
    fprintf(data_file, "-- local chunk = loadstring(data)\n");
    fprintf(data_file, "-- if chunk then chunk() end\n");
    
    fclose(data_file);
    LOGD("Saved match data to: %s", file_path);
    return 0;
}

/**
 * 释放内存区域列表
 * @param regions 内存区域列表
 * @param count 区域数量
 */
static void free_memory_maps(mem_region_t *regions, size_t count) {
    if (regions) {
        free(regions);
    }
}

/**
 * 扫描单个内存区域
 * @param start 区域起始地址
 * @param end 区域结束地址
 * @param L Lua状态机
 * @param file 文件指针，用于写入扫描结果
 */
static void scan_memory_region(uintptr_t start, uintptr_t end, lua_State *L, FILE *file) {
    size_t region_size = end - start;
    if (region_size == 0) return;

    // 映射内存到当前进程
    void *mapped = (void *)start;
    if (mapped == NULL) {
        return;
    }

    // 扫描LuaS魔数和特殊字符串
    for (size_t i = 0; i < region_size - 3; i++) {
        const uint8_t *ptr = (const uint8_t *)mapped + i;
        bool found = false;
        int type = 0;
        const char *type_name = NULL;
        size_t match_size = 0;
        
        // 检查Lua字节码签名
        if (memcmp(ptr, MY_LUA_SIGNATURE, sizeof(MY_LUA_SIGNATURE)) == 0) {
            found = true;
            type = 1;
            type_name = "Lua Bytecode";
            match_size = sizeof(MY_LUA_SIGNATURE);
        }
        
        // 检查LuaJIT字节码签名
        else if (memcmp(ptr, MY_LUAJIT_SIGNATURE, sizeof(MY_LUAJIT_SIGNATURE)) == 0) {
            found = true;
            type = 2;
            type_name = "LuaJIT Bytecode";
            match_size = sizeof(MY_LUAJIT_SIGNATURE);
        }
        
        // 检查=IA, =IB, =IC, =ID开头的字符串
        else if (i + 3 < region_size) {
            if (ptr[0] == '=' && (ptr[1] == 'I' || ptr[1] == 'i') && 
                (ptr[2] == 'A' || ptr[2] == 'B' || ptr[2] == 'C' || ptr[2] == 'D')) {
                found = true;
                type = 3;
                type_name = "Special String";
                match_size = 3;
            }
        }
        
        if (found) {
            // 找到匹配，添加到结果表
            lua_pushlightuserdata(L, (void *)(start + i));
            lua_pushinteger(L, type);
            lua_settable(L, -3);
            
            // 写入到文件
            if (file) {
                fprintf(file, "-- 地址: 0x%lx, 类型: %s\n", (start + i), type_name);
                
                // 读取并写入更多内容（最多1024字节）
                size_t read_size = region_size - i;
                if (read_size > 81920) {
                    read_size = 81920;
                }
                
                // 写入原始数据（十六进制格式）
                fprintf(file, "local data = '\\x");
                for (size_t j = 0; j < read_size; j++) {
                    fprintf(file, "%02x", ptr[j]);
                }
                fprintf(file, "'\n\n");
                
                // 保存匹配的数据到单独的文件
                save_match_data(ptr, read_size, (start + i), type_name);
            }
        }
    }
}

/**
 * 执行内存扫描的主函数
 * @param L Lua状态机
 * @return 1表示返回一个结果表
 */
static int l_scan_memory(lua_State *L) {
    LOGD("Starting memory scan...");
    
    // 创建结果表
    lua_newtable(L);
    
    // 创建输出目录和Data子目录
    const char *base_dir = "/sdcard/XCLUA/Mem";
    const char *data_dir = "/sdcard/XCLUA/Mem/Data";
    if (mkdir_recursive(base_dir, 0755) != 0 || 
        mkdir_recursive(data_dir, 0755) != 0) {
        LOGE("Failed to create output directory: %s", strerror(errno));
        return 1;
    }
    
    // 生成文件名
    char date_str[11];
    char time_str[9];
    char file_path[256];
    
    if (get_current_date(date_str, sizeof(date_str)) == 0 || 
        get_current_time(time_str, sizeof(time_str)) == 0) {
        LOGE("Failed to get current date/time");
        return 1;
    }
    
    // 生成完整的文件路径：/sdcard/XCLUA/Mem/日期_时间.lua
    snprintf(file_path, sizeof(file_path), "%s/%s_%s.lua", base_dir, date_str, time_str);
    
    // 打开文件
    FILE *file = fopen(file_path, "w");
    if (!file) {
        LOGE("Failed to open output file: %s", strerror(errno));
        return 1;
    }
    
    // 写入文件头
    fprintf(file, "-- Lua内存扫描结果\n");
    fprintf(file, "-- 扫描时间: %s %s\n", date_str, time_str);
    fprintf(file, "-- 输出目录: %s\n\n", base_dir);
    
    // 读取内存映射
    mem_region_t *regions = NULL;
    size_t region_count = 0;
    
    if (read_memory_maps(&regions, &region_count) != 0) {
        fclose(file);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to read memory maps");
        return 2;
    }
    
    LOGD("Found %zu memory regions", region_count);
    fprintf(file, "-- 扫描的内存区域数量: %zu\n\n", region_count);
    
    // 扫描每个内存区域
    for (size_t i = 0; i < region_count; i++) {
        mem_region_t *r = &regions[i];
        
        // 跳过不可读区域
        if (strchr(r->permissions, 'r') == NULL) {
            continue;
        }
        
        // 跳过空区域
        if (r->start >= r->end) {
            continue;
        }
        
        // 写入当前区域信息
        fprintf(file, "-- 扫描区域: 0x%lx-0x%lx, 权限: %s, 名称: %s\n", 
                r->start, r->end, r->permissions, r->name);
        
        // 执行扫描
        scan_memory_region(r->start, r->end, L, file);
    }
    
    // 写入文件尾
    fprintf(file, "-- 扫描完成\n");
    fclose(file);
    
    // 释放内存
    free_memory_maps(regions, region_count);
    
    LOGD("Memory scan completed, results saved to: %s", file_path);
    
    // 将文件名添加到返回结果的元表中
    lua_createtable(L, 0, 1);
    lua_pushstring(L, file_path);
    lua_setfield(L, -2, "file_path");
    lua_setmetatable(L, -2);
    
    return 1;
}

/**
 * 从指定地址读取数据
 * @param L Lua状态机
 * @return 2表示返回数据和大小
 */
static int l_read_memory(lua_State *L) {
    uintptr_t addr = (uintptr_t)lua_touserdata(L, 1);
    size_t size = luaL_checkinteger(L, 2);
    
    if (size == 0) {
        lua_pushstring(L, "");
        lua_pushinteger(L, 0);
        return 2;
    }
    
    // 分配缓冲区
    char *buffer = malloc(size + 1);
    if (!buffer) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to allocate buffer");
        return 2;
    }
    
    // 读取内存
    memcpy(buffer, (const void *)addr, size);
    buffer[size] = '\0';
    
    lua_pushlstring(L, buffer, size);
    lua_pushinteger(L, size);
    free(buffer);
    
    return 2;
}

/**
 * 保存内存数据到文件
 * @param L Lua状态机
 * @return 1表示返回布尔值
 */
static int l_save_memory(lua_State *L) {
    uintptr_t addr = (uintptr_t)lua_touserdata(L, 1);
    size_t size = luaL_checkinteger(L, 2);
    const char *file_path = luaL_checkstring(L, 3);
    
    FILE *fp = fopen(file_path, "wb");
    if (!fp) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, strerror(errno));
        return 2;
    }
    
    size_t written = fwrite((const void *)addr, 1, size, fp);
    fclose(fp);
    
    if (written != size) {
        lua_pushboolean(L, 0);
        lua_pushstring(L, "Failed to write complete data");
        return 2;
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

// 模块方法列表
static const luaL_Reg memhack_lib[] = {
    {"scan", l_scan_memory},
    {"read", l_read_memory},
    {"save", l_save_memory},
    {NULL, NULL} // 结束标记
};

/**
 * 模块初始化函数
 * @param L Lua状态机
 * @return 1表示返回模块表
 */
int luaopen_memhack(lua_State *L) {
    LOGD("Initializing memhack module");
    
    luaL_newlib(L, memhack_lib);
    
    // 添加常量
    lua_pushinteger(L, 1);
    lua_setfield(L, -2, "TYPE_LUA_BYTECODE");
    
    lua_pushinteger(L, 2);
    lua_setfield(L, -2, "TYPE_LUAJIT_BYTECODE");
    
    lua_pushinteger(L, 3);
    lua_setfield(L, -2, "TYPE_SPECIAL_STRING");
    
    return 1;
}
