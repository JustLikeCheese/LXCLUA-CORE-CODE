#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

// 函数声明
static int has_root(lua_State *L);
static int execute_root_command(lua_State *L);
static int read_root_file(lua_State *L);
static int write_root_file(lua_State *L);
static int mount_file_system(lua_State *L);
static int umount_file_system(lua_State *L);
static int chmod_root(lua_State *L);
static int chown_root(lua_State *L);

/**
 * 检查设备是否具有 root 权限
 * @param L Lua 状态机
 * @return 1 表示具有 root 权限，0 表示没有
 */
static int has_root(lua_State *L) {
    // 检查 su 命令是否存在
    FILE *fp = popen("which su", "r");
    if (fp == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    char buffer[256];
    size_t read_size = fread(buffer, 1, sizeof(buffer), fp);
    pclose(fp);
    
    if (read_size > 0) {
        // 尝试执行简单的 su 命令来验证权限
        fp = popen("su -c 'id' 2>/dev/null", "r");
        if (fp != NULL) {
            read_size = fread(buffer, 1, sizeof(buffer), fp);
            pclose(fp);
            
            if (read_size > 0 && strstr(buffer, "uid=0") != NULL) {
                lua_pushboolean(L, 1);
                return 1;
            }
        }
    }
    
    lua_pushboolean(L, 0);
    return 1;
}

/**
 * 执行 root 命令
 * @param L Lua 状态机
 * @return 2 返回命令输出和退出码
 */
static int execute_root_command(lua_State *L) {
    const char *command = luaL_checkstring(L, 1);
    
    // 构建 su 命令
    char su_command[4096];
    snprintf(su_command, sizeof(su_command), "su -c '%s'", command);
    
    FILE *fp = popen(su_command, "r");
    if (fp == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, strerror(errno));
        return 2;
    }
    
    // 读取命令输出
    char buffer[4096];
    size_t read_size;
    luaL_Buffer output;
    luaL_buffinit(L, &output);
    
    while ((read_size = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        luaL_addlstring(&output, buffer, read_size);
    }
    
    int exit_code = pclose(fp);
    luaL_pushresult(&output);
    
    // 返回退出码
    lua_pushinteger(L, WEXITSTATUS(exit_code));
    
    return 2;
}

/**
 * 以 root 权限读取文件内容
 * @param L Lua 状态机
 * @return 2 返回文件内容和错误信息
 */
static int read_root_file(lua_State *L) {
    const char *file_path = luaL_checkstring(L, 1);
    
    // 构建 cat 命令
    char command[4096];
    snprintf(command, sizeof(command), "su -c 'cat %s'", file_path);
    
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, strerror(errno));
        return 2;
    }
    
    // 读取文件内容
    char buffer[4096];
    size_t read_size;
    luaL_Buffer content;
    luaL_buffinit(L, &content);
    
    while ((read_size = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        luaL_addlstring(&content, buffer, read_size);
    }
    
    int exit_code = pclose(fp);
    
    if (exit_code != 0) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to read file with root permission");
        return 2;
    }
    
    luaL_pushresult(&content);
    lua_pushnil(L); // 无错误
    
    return 2;
}

/**
 * 以 root 权限写入文件内容
 * @param L Lua 状态机
 * @return 1 返回是否成功
 */
static int write_root_file(lua_State *L) {
    const char *file_path = luaL_checkstring(L, 1);
    const char *content = luaL_checkstring(L, 2);
    
    // 构建 echo 命令写入文件
    char command[4096];
    snprintf(command, sizeof(command), "su -c 'echo -n \"%s\" > %s'", content, file_path);
    
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    int exit_code = pclose(fp);
    lua_pushboolean(L, exit_code == 0);
    
    return 1;
}

/**
 * 以 root 权限挂载文件系统
 * @param L Lua 状态机
 * @return 1 返回是否成功
 */
static int mount_file_system(lua_State *L) {
    const char *source = luaL_checkstring(L, 1);
    const char *target = luaL_checkstring(L, 2);
    const char *filesystem_type = luaL_optstring(L, 3, "");
    int flags = luaL_optinteger(L, 4, 0);
    
    // 构建 mount 命令
    char command[4096];
    if (strlen(filesystem_type) > 0) {
        snprintf(command, sizeof(command), "su -c 'mount -t %s -o %s %s %s'", 
                 filesystem_type, flags == MS_RDONLY ? "ro" : "rw", source, target);
    } else {
        snprintf(command, sizeof(command), "su -c 'mount -o %s %s %s'", 
                 flags == MS_RDONLY ? "ro" : "rw", source, target);
    }
    
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    int exit_code = pclose(fp);
    lua_pushboolean(L, exit_code == 0);
    
    return 1;
}

/**
 * 以 root 权限卸载文件系统
 * @param L Lua 状态机
 * @return 1 返回是否成功
 */
static int umount_file_system(lua_State *L) {
    const char *target = luaL_checkstring(L, 1);
    
    // 构建 umount 命令
    char command[4096];
    snprintf(command, sizeof(command), "su -c 'umount %s'", target);
    
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    int exit_code = pclose(fp);
    lua_pushboolean(L, exit_code == 0);
    
    return 1;
}

/**
 * 以 root 权限修改文件权限
 * @param L Lua 状态机
 * @return 1 返回是否成功
 */
static int chmod_root(lua_State *L) {
    const char *file_path = luaL_checkstring(L, 1);
    int mode = luaL_checkinteger(L, 2);
    
    // 构建 chmod 命令
    char command[4096];
    snprintf(command, sizeof(command), "su -c 'chmod %o %s'", mode, file_path);
    
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    int exit_code = pclose(fp);
    lua_pushboolean(L, exit_code == 0);
    
    return 1;
}

/**
 * 以 root 权限修改文件所有者
 * @param L Lua 状态机
 * @return 1 返回是否成功
 */
static int chown_root(lua_State *L) {
    const char *file_path = luaL_checkstring(L, 1);
    int uid = luaL_checkinteger(L, 2);
    int gid = luaL_checkinteger(L, 3);
    
    // 构建 chown 命令
    char command[4096];
    snprintf(command, sizeof(command), "su -c 'chown %d:%d %s'", uid, gid, file_path);
    
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        lua_pushboolean(L, 0);
        return 1;
    }
    
    int exit_code = pclose(fp);
    lua_pushboolean(L, exit_code == 0);
    
    return 1;
}

// 模块函数列表
static const luaL_Reg root_lib[] = {
    {"hasroot", has_root},
    {"execute", execute_root_command},
    {"readfile", read_root_file},
    {"writefile", write_root_file},
    {"mount", mount_file_system},
    {"umount", umount_file_system},
    {"chmod", chmod_root},
    {"chown", chown_root},
    {NULL, NULL}
};

/**
 * 模块注册函数
 * @param L Lua 状态机
 */
int luaopen_root(lua_State *L) {
    luaL_newlib(L, root_lib);
    return 1;
}