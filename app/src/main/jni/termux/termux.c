#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define TERMUX_UNUSED(x) x __attribute__((__unused__))
#ifdef __APPLE__
# define LACKS_PTSNAME_R
#endif

/**
 * 抛出 Lua 错误
 * @param L Lua 状态指针
 * @param message 错误信息
 * @return 错误码
 */
static int throw_lua_error(lua_State* L, const char* message)
{
    lua_pushstring(L, message);
    return lua_error(L);
}

/**
 * 创建子进程并返回主端文件描述符
 * @param L Lua 状态指针
 * @param cmd 要执行的命令
 * @param cwd 工作目录
 * @param argv 命令参数数组
 * @param envp 环境变量数组
 * @param pProcessId 输出参数，用于存储子进程ID
 * @return 主端文件描述符
 */
static int create_subprocess(lua_State* L,
        const char* cmd,
        const char* cwd,
        char* const argv[],
        char** envp,
        int* pProcessId)
{
    int ptm = open("/dev/ptmx", O_RDWR | O_CLOEXEC);
    if (ptm < 0) return throw_lua_error(L, "Cannot open /dev/ptmx");

#ifdef LACKS_PTSNAME_R
    char* devname;
#else
    char devname[64];
#endif
    if (grantpt(ptm) || unlockpt(ptm) ||
#ifdef LACKS_PTSNAME_R
            (devname = ptsname(ptm)) == NULL
#else
            ptsname_r(ptm, devname, sizeof(devname))
#endif
       ) {
        return throw_lua_error(L, "Cannot grantpt()/unlockpt()/ptsname_r() on /dev/ptmx");
    }

    // Enable UTF-8 mode and disable flow control to prevent Ctrl+S from locking up the display.
    struct termios tios;
    tcgetattr(ptm, &tios);
    
    // Set terminal to raw mode to disable all processing
    tios.c_iflag = 0;                  // Input flags
    tios.c_oflag = 0;                  // Output flags
    tios.c_lflag = 0;                  // Local flags
    tios.c_cflag = CS8 | CREAD | CLOCAL; // Control flags: 8 bits, enable receiver, ignore modem status
    
    // Set VMIN and VTIME for non-blocking read
    tios.c_cc[VMIN] = 1;               // Minimum number of characters to read
    tios.c_cc[VTIME] = 0;              // Timeout in deciseconds
    
    // Enable UTF-8 mode
    tios.c_iflag |= IUTF8;
    
    tcsetattr(ptm, TCSANOW, &tios);

    /** Set initial winsize with default values. */
    struct winsize sz = { .ws_row = 24, .ws_col = 80, .ws_xpixel = 640, .ws_ypixel = 384 };
    ioctl(ptm, TIOCSWINSZ, &sz);

    pid_t pid = fork();
    if (pid < 0) {
        return throw_lua_error(L, "Fork failed");
    } else if (pid > 0) {
        *pProcessId = (int) pid;
        return ptm;
    } else {
        // Clear signals which the Android java process may have blocked:
        sigset_t signals_to_unblock;
        sigfillset(&signals_to_unblock);
        sigprocmask(SIG_UNBLOCK, &signals_to_unblock, 0);

        close(ptm);
        setsid();

        int pts = open(devname, O_RDWR);
        if (pts < 0) exit(-1);

        dup2(pts, 0);
        dup2(pts, 1);
        dup2(pts, 2);

        DIR* self_dir = opendir("/proc/self/fd");
        if (self_dir != NULL) {
            int self_dir_fd = dirfd(self_dir);
            struct dirent* entry;
            while ((entry = readdir(self_dir)) != NULL) {
                int fd = atoi(entry->d_name);
                if (fd > 2 && fd != self_dir_fd) close(fd);
            }
            closedir(self_dir);
        }

        clearenv();
        if (envp) for (; *envp; ++envp) putenv(*envp);

        if (chdir(cwd) != 0) {
            char* error_message;
            // No need to free asprintf()-allocated memory since doing execvp() or exit() below.
            if (asprintf(&error_message, "chdir(\"%s\")", cwd) == -1) error_message = "chdir()";
            perror(error_message);
            fflush(stderr);
        }
        execvp(cmd, argv);
        // Show terminal output about failing exec() call:
        char* error_message;
        if (asprintf(&error_message, "exec(\"%s\")", cmd) == -1) error_message = "exec()";
        perror(error_message);
        _exit(1);
    }
}

/**
 * 创建子进程的 Lua 接口函数
 * @param L Lua 状态指针
 * @return 返回值数量，返回文件描述符和进程ID
 * @note Lua 参数：
 *       1. cmd: 要执行的命令路径
 *       2. cwd: 工作目录
 *       3. argv: 命令参数表（可选）
 *       4. envp: 环境变量表（可选）
 */
static int l_create(lua_State* L)
{
    const char* cmd = luaL_checkstring(L, 1);
    const char* cwd = luaL_checkstring(L, 2);
    
    // 解析参数表
    char** argv = NULL;
    int argc = 0;
    if (lua_istable(L, 3)) {
        argc = lua_rawlen(L, 3);
        argv = (char**)malloc((argc + 1) * sizeof(char*));
        if (!argv) return throw_lua_error(L, "Couldn't allocate argv array");
        for (int i = 0; i < argc; ++i) {
            lua_rawgeti(L, 3, i + 1);
            const char* arg = luaL_checkstring(L, -1);
            argv[i] = strdup(arg);
            lua_pop(L, 1);
        }
        argv[argc] = NULL;
    }
    
    // 解析环境变量表
    char** envp = NULL;
    int envc = 0;
    if (lua_istable(L, 4)) {
        envc = lua_rawlen(L, 4);
        envp = (char**)malloc((envc + 1) * sizeof(char*));
        if (!envp) return throw_lua_error(L, "malloc() for envp array failed");
        for (int i = 0; i < envc; ++i) {
            lua_rawgeti(L, 4, i + 1);
            const char* env = luaL_checkstring(L, -1);
            envp[i] = strdup(env);
            lua_pop(L, 1);
        }
        envp[envc] = NULL;
    }
    
    int procId = 0;
    int ptm = create_subprocess(L, cmd, cwd, argv, envp, &procId);
    
    // 释放内存
    if (argv) {
        for (char** tmp = argv; *tmp; ++tmp) free(*tmp);
        free(argv);
    }
    if (envp) {
        for (char** tmp = envp; *tmp; ++tmp) free(*tmp);
        free(envp);
    }
    
    // 返回文件描述符和进程ID
    lua_pushinteger(L, ptm);
    lua_pushinteger(L, procId);
    return 2;
}

/**
 * 设置终端为 UTF-8 模式
 * @param L Lua 状态指针
 * @return 返回值数量，无返回值
 * @note Lua 参数：
 *       1. fd: 文件描述符
 */
static int l_setutf8mode(lua_State* L)
{
    int fd = luaL_checkinteger(L, 1);
    
    struct termios tios;
    tcgetattr(fd, &tios);
    if ((tios.c_iflag & IUTF8) == 0) {
        tios.c_iflag |= IUTF8;
        tcsetattr(fd, TCSANOW, &tios);
    }
    
    return 0;
}

/**
 * 等待子进程结束
 * @param L Lua 状态指针
 * @return 返回值数量，返回子进程退出码
 * @note Lua 参数：
 *       1. pid: 子进程ID
 * @note 返回值：
 *       - 正常退出：返回退出码
 *       - 被信号终止：返回负的信号编号
 *       - 其他情况：返回0
 */
static int l_wait(lua_State* L)
{
    int pid = luaL_checkinteger(L, 1);
    
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        lua_pushinteger(L, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        lua_pushinteger(L, -WTERMSIG(status));
    } else {
        // Should never happen - waitpid(2) says "One of the first three macros will evaluate to a non-zero (true) value".
        lua_pushinteger(L, 0);
    }
    
    return 1;
}

/**
 * 关闭文件描述符
 * @param L Lua 状态指针
 * @return 返回值数量，无返回值
 * @note Lua 参数：
 *       1. fd: 文件描述符
 */
static int l_close(lua_State* L)
{
    int fd = luaL_checkinteger(L, 1);
    close(fd);
    return 0;
}

/**
 * 从文件描述符读取数据
 * @param L Lua 状态指针
 * @return 返回值数量，返回读取的数据
 * @note Lua 参数：
 *       1. fd: 文件描述符
 *       2. size: 读取大小（可选，默认4096）
 * @note 改进：使用非阻塞读取，尝试读取直到遇到提示符或超时
 */
static int l_read(lua_State* L)
{
    int fd = luaL_checkinteger(L, 1);
    size_t size = luaL_optinteger(L, 2, 4096);
    
    char* buffer = (char*)malloc(size);
    if (!buffer) return throw_lua_error(L, "malloc() for read buffer failed");
    
    // 设置文件描述符为非阻塞模式
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    
    ssize_t total_read = 0;
    int timeout = 0;
    const int max_timeout = 100; // 最大尝试次数
    
    while (total_read < size && timeout < max_timeout) {
        ssize_t nread = read(fd, buffer + total_read, size - total_read);
        if (nread > 0) {
            total_read += nread;
            timeout = 0; // 重置超时计数器
        } else if (nread < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            free(buffer);
            return throw_lua_error(L, "read() failed");
        } else {
            // 没有数据可读，等待一段时间
            usleep(10000); // 10ms
            timeout++;
        }
    }
    
    // 恢复文件描述符为阻塞模式
    fcntl(fd, F_SETFL, flags);
    
    lua_pushlstring(L, buffer, total_read);
    free(buffer);
    return 1;
}

/**
 * 向文件描述符写入数据
 * @param L Lua 状态指针
 * @return 返回值数量，返回写入的字节数
 * @note Lua 参数：
 *       1. fd: 文件描述符
 *       2. data: 要写入的数据
 */
static int l_write(lua_State* L)
{
    int fd = luaL_checkinteger(L, 1);
    size_t len;
    const char* data = luaL_checklstring(L, 2, &len);
    
    ssize_t nwritten = write(fd, data, len);
    if (nwritten < 0) {
        return throw_lua_error(L, "write() failed");
    }
    
    lua_pushinteger(L, nwritten);
    return 1;
}

static const struct luaL_Reg term_lib[] = {
    {"create", l_create},
    {"setutf8mode", l_setutf8mode},
    {"wait", l_wait},
    {"close", l_close},
    {"read", l_read},
    {"write", l_write},
    {NULL, NULL}
};

/**
 * Lua 模块入口函数，注册 termux 模块
 * @param L Lua 状态指针
 * @return 返回值数量，返回创建的模块
 */
int luaopen_termux(lua_State* L)
{
    luaL_newlib(L, term_lib);
    return 1;
}
