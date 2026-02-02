#include "LuaBoost.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>

#if defined(__ANDROID__)
#define CPU_SETSIZE 1024
#define __NCPUBITS (8 * sizeof(unsigned long))

typedef struct {
    unsigned long __bits[CPU_SETSIZE / __NCPUBITS];
} cpu_set_t;

#define CPU_ZERO(cpuset) do { memset(cpuset, 0, sizeof(cpu_set_t)); } while(0)
#define CPU_SET(cpu, cpuset) \
    ((cpuset)->__bits[(cpu)/__NCPUBITS] |= (1UL << ((cpu) % __NCPUBITS)))
#define CPU_CLR(cpu, cpuset) \
    ((cpuset)->__bits[(cpu)/__NCPUBITS] &= ~(1UL << ((cpu) % __NCPUBITS)))
#define CPU_ISSET(cpu, cpuset) \
    (((cpuset)->__bits[(cpu)/__NCPUBITS] >> ((cpu) % __NCPUBITS)) & 1)
#define CPU_COUNT(cpuset) __builtin_popcountll(*(unsigned long*)(cpuset))

static int sched_setaffinity(pid_t pid, size_t cpusetsize, cpu_set_t* cpuset) {
    return syscall(__NR_sched_setaffinity, pid, cpusetsize, cpuset);
}

static int sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t* cpuset) {
    return syscall(__NR_sched_getaffinity, pid, cpusetsize, cpuset);
}

static int sched_getcpu(void) {
    unsigned long cpu;
    if (syscall(__NR_getcpu, &cpu, NULL, NULL) == 0) {
        return (int)cpu;
    }
    return -1;
}
#endif

static pthread_mutex_t g_cpu_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_freq_mutex = PTHREAD_MUTEX_INITIALIZER;
static int64_t g_start_time_ms = 0;

cpu_manager_t g_cpu_manager;
boost_config_t g_boost_config;

static int64_t get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static int snprintf_path(char* buf, size_t size, const char* fmt, int cpu_id) {
    return snprintf(buf, size, fmt, cpu_id);
}

static int read_sysfs_int(const char* path, int* value) {
    FILE* fp = fopen(path, "r");
    if (!fp) return -1;
    int result = fscanf(fp, "%d", value);
    fclose(fp);
    return result == 1 ? 0 : -1;
}

static int read_sysfs_long(const char* path, long* value) {
    FILE* fp = fopen(path, "r");
    if (!fp) return -1;
    int result = fscanf(fp, "%ld", value);
    fclose(fp);
    return result == 1 ? 0 : -1;
}

static int write_sysfs_int(const char* path, int value) {
    FILE* fp = fopen(path, "w");
    if (!fp) return -1;
    int result = fprintf(fp, "%d", value);
    fclose(fp);
    return result > 0 ? 0 : -1;
}

static int read_cpu_info_fast(cpu_info_t* info, int cpu_id) {
    char path[MAX_PATH];
    
    snprintf_path(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpu_capacity", cpu_id);
    read_sysfs_int(path, &info->cpu_capacity);
    
    snprintf_path(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/online", cpu_id);
    read_sysfs_int(path, &info->online);
    
    snprintf_path(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cpu_id);
    read_sysfs_long(path, &info->max_freq);
    
    snprintf_path(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", cpu_id);
    read_sysfs_long(path, &info->min_freq);
    
    snprintf_path(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", cpu_id);
    read_sysfs_long(path, &info->cur_freq);
    
    return 0;
}

void lua_boost_init(void) {
    pthread_mutex_lock(&g_cpu_mutex);
    
    if (g_cpu_manager.initialized) {
        pthread_mutex_unlock(&g_cpu_mutex);
        return;
    }
    
    memset(&g_cpu_manager, 0, sizeof(g_cpu_manager));
    memset(&g_boost_config, 0, sizeof(g_boost_config));
    
    g_start_time_ms = get_time_ms();
    
    DIR* dir;
    char path[MAX_PATH];
    int cpu_count = 0;
    
    for (int i = 0; i < MAX_CPU; i++) {
        snprintf_path(path, sizeof(path), "/sys/devices/system/cpu/cpu%d", i);
        dir = opendir(path);
        if (dir) {
            closedir(dir);
            cpu_info_t* info = &g_cpu_manager.cpu_info[i];
            memset(info, 0, sizeof(cpu_info_t));
            info->cpu_id = i;
            info->valid = 1;
            cpu_count++;
            
            read_cpu_info_fast(info, i);
        }
    }
    
    g_cpu_manager.cpu_count = cpu_count;
    
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        char line[256];
        int current_cpu = -1;
        int found_processor = 0;
        
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "processor", 9) == 0) {
                char* colon = strchr(line, ':');
                if (colon) {
                    current_cpu = atoi(colon + 2);
                    if (current_cpu >= 0 && current_cpu < MAX_CPU) {
                        g_cpu_manager.cpu_info[current_cpu].cpu_id = current_cpu;
                        found_processor = 1;
                    }
                }
            } else if (found_processor) {
                if (strncmp(line, "BogoMIPS", 8) == 0) {
                    char* colon = strchr(line, ':');
                    if (colon) {
                        g_cpu_manager.cpu_info[current_cpu].bogomips = atof(colon + 2);
                    }
                } else if (strncmp(line, "CPU implementer", 15) == 0) {
                    char* colon = strchr(line, ':');
                    if (colon) {
                        g_cpu_manager.cpu_info[current_cpu].implementer = strtol(colon + 2, NULL, 16);
                    }
                } else if (strncmp(line, "CPU architecture", 16) == 0) {
                    char* colon = strchr(line, ':');
                    if (colon) {
                        g_cpu_manager.cpu_info[current_cpu].architecture = atoi(colon + 2);
                    }
                } else if (strncmp(line, "CPU variant", 11) == 0) {
                    char* colon = strchr(line, ':');
                    if (colon) {
                        g_cpu_manager.cpu_info[current_cpu].variant = strtol(colon + 2, NULL, 16);
                    }
                } else if (strncmp(line, "CPU part", 8) == 0) {
                    char* colon = strchr(line, ':');
                    if (colon) {
                        g_cpu_manager.cpu_info[current_cpu].part = strtol(colon + 2, NULL, 16);
                    }
                } else if (strncmp(line, "CPU revision", 12) == 0) {
                    char* colon = strchr(line, ':');
                    if (colon) {
                        g_cpu_manager.cpu_info[current_cpu].revision = atoi(colon + 2);
                    }
                }
            }
        }
        fclose(fp);
    }
    
    int capacities[MAX_CPU];
    int cap_count = 0;
    for (int i = 0; i < MAX_CPU; i++) {
        if (g_cpu_manager.cpu_info[i].valid) {
            int cap = g_cpu_manager.cpu_info[i].cpu_capacity;
            if (cap > 0) {
                int found = 0;
                for (int j = 0; j < cap_count; j++) {
                    if (capacities[j] == cap) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    capacities[cap_count++] = cap;
                }
            }
        }
    }
    
    for (int i = 0; i < cap_count; i++) {
        for (int j = i + 1; j < cap_count; j++) {
            if (capacities[j] > capacities[i]) {
                int temp = capacities[i];
                capacities[i] = capacities[j];
                capacities[j] = temp;
            }
        }
    }
    
    g_cpu_manager.max_capacity = (cap_count > 0) ? capacities[0] : 0;
    g_cpu_manager.min_capacity = (cap_count > 0) ? capacities[cap_count - 1] : 0;
    g_cpu_manager.capacity_count = cap_count;
    
    for (int i = 0; i < cap_count && i < MAX_CPU; i++) {
        g_cpu_manager.unique_capacities[i] = capacities[i];
    }
    
    for (int i = 0; i < MAX_CPU; i++) {
        if (g_cpu_manager.cpu_info[i].valid) {
            if (g_cpu_manager.cpu_info[i].cpu_capacity == g_cpu_manager.max_capacity) {
                g_cpu_manager.cpu_info[i].is_big_core = 1;
                g_cpu_manager.big_core_count++;
            } else {
                g_cpu_manager.little_core_count++;
            }
        }
    }
    
    g_cpu_manager.last_update_time = get_time_ms();
    g_cpu_manager.initialized = 1;
    
    pthread_mutex_unlock(&g_cpu_mutex);
}

void lua_boost_shutdown(void) {
    pthread_mutex_lock(&g_cpu_mutex);
    g_cpu_manager.initialized = 0;
    pthread_mutex_unlock(&g_cpu_mutex);
}

int lua_boost_get_cpu_count(void) {
    return g_cpu_manager.cpu_count;
}

int lua_boost_get_big_core_count(void) {
    return g_cpu_manager.big_core_count;
}

int lua_boost_is_big_core(int cpu_id) {
    if (cpu_id >= 0 && cpu_id < MAX_CPU) {
        return g_cpu_manager.cpu_info[cpu_id].is_big_core;
    }
    return 0;
}

int lua_boost_get_max_capacity(void) {
    return g_cpu_manager.max_capacity;
}

int lua_boost_get_min_capacity(void) {
    return g_cpu_manager.min_capacity;
}

int lua_boost_set_cpu_affinity(int* cpu_list, int count) {
    if (!cpu_list || count <= 0) return -1;
    
    cpu_set_t mask;
    CPU_ZERO(&mask);
    
    for (int i = 0; i < count; i++) {
        if (cpu_list[i] >= 0 && cpu_list[i] < MAX_CPU) {
            CPU_SET(cpu_list[i], &mask);
        }
    }
    
    pid_t pid = getpid();
    return sched_setaffinity(pid, sizeof(cpu_set_t), &mask);
}

int lua_boost_set_cpu_frequency(int cpu_id, long freq) {
    (void)cpu_id;
    (void)freq;
    return -1;
}

long lua_boost_get_current_frequency(int cpu_id) {
    if (cpu_id < 0 || cpu_id >= MAX_CPU) return -1;
    return g_cpu_manager.cpu_info[cpu_id].cur_freq;
}

static int update_cpu_info_cache(void) {
    int64_t current_time = get_time_ms();
    
    if (current_time - g_cpu_manager.last_update_time < CPU_CACHE_TIMEOUT_MS) {
        return 0;
    }
    
    for (int i = 0; i < MAX_CPU; i++) {
        if (g_cpu_manager.cpu_info[i].valid) {
            read_cpu_info_fast(&g_cpu_manager.cpu_info[i], i);
        }
    }
    
    g_cpu_manager.last_update_time = current_time;
    return 0;
}

static int set_affinity_by_capacity(int capacity, int use_big_cores) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    
    for (int i = 0; i < MAX_CPU; i++) {
        if (g_cpu_manager.cpu_info[i].valid) {
            if (capacity > 0) {
                if (use_big_cores) {
                    if (g_cpu_manager.cpu_info[i].cpu_capacity == capacity && 
                        g_cpu_manager.cpu_info[i].is_big_core) {
                        CPU_SET(i, &mask);
                    }
                } else {
                    if (g_cpu_manager.cpu_info[i].cpu_capacity == capacity) {
                        CPU_SET(i, &mask);
                    }
                }
            } else {
                if (g_cpu_manager.cpu_info[i].online > 0) {
                    CPU_SET(i, &mask);
                }
            }
        }
    }
    
    pid_t pid = getpid();
    return sched_setaffinity(pid, sizeof(cpu_set_t), &mask);
}

static int set_affinity_by_mode(performance_mode_t mode) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    
    int use_big_cores = 0;
    int use_little_cores = 0;
    int core_count = 0;
    
    switch (mode) {
        case PERFORMANCE_MODE_LOW_POWER:
            use_big_cores = 0;
            use_little_cores = 1;
            core_count = 1;
            break;
            
        case PERFORMANCE_MODE_LOW_BOOST:
            use_big_cores = 1;
            use_little_cores = 0;
            core_count = 1;
            break;
            
        case PERFORMANCE_MODE_BALANCED:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = 2;
            break;
            
        case PERFORMANCE_MODE_MIDDLE_BOOST:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = g_cpu_manager.big_core_count + 2;
            break;
            
        case PERFORMANCE_MODE_BOOST:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = g_cpu_manager.big_core_count + g_cpu_manager.little_core_count / 3;
            break;
            
        case PERFORMANCE_MODE_HIGH_BOOST:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = g_cpu_manager.big_core_count + g_cpu_manager.little_core_count * 2 / 3;
            break;
            
        case PERFORMANCE_MODE_TURBO:
            use_big_cores = 1;
            use_little_cores = 0;
            core_count = g_cpu_manager.big_core_count;
            break;
            
        case PERFORMANCE_MODE_PERFORMANCE:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = g_cpu_manager.big_core_count + g_cpu_manager.little_core_count;
            break;
            
        case PERFORMANCE_MODE_DYNAMIC:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = g_cpu_manager.big_core_count + g_cpu_manager.little_core_count;
            break;
            
        default:
            return -1;
    }
    
    int big_core_count = 0;
    int little_core_count = 0;
    
    if (use_big_cores) {
        for (int i = 0; i < MAX_CPU && big_core_count < g_cpu_manager.big_core_count && big_core_count < core_count; i++) {
            if (g_cpu_manager.cpu_info[i].valid && g_cpu_manager.cpu_info[i].is_big_core) {
                CPU_SET(i, &mask);
                big_core_count++;
            }
        }
    }
    
    if (use_little_cores && core_count > big_core_count) {
        int needed_little = core_count - big_core_count;
        for (int i = 0; i < MAX_CPU && little_core_count < needed_little; i++) {
            if (g_cpu_manager.cpu_info[i].valid && !g_cpu_manager.cpu_info[i].is_big_core) {
                CPU_SET(i, &mask);
                little_core_count++;
            }
        }
    }
    
    if (CPU_COUNT(&mask) == 0) {
        for (int i = 0; i < MAX_CPU && core_count > 0; i++) {
            if (g_cpu_manager.cpu_info[i].valid) {
                CPU_SET(i, &mask);
                core_count--;
            }
        }
    }
    
    pid_t pid = getpid();
    return sched_setaffinity(pid, sizeof(cpu_set_t), &mask);
}

int lua_boost_set_performance_mode(performance_mode_t mode) {
    pthread_mutex_lock(&g_cpu_mutex);
    update_cpu_info_cache();
    
    int result = set_affinity_by_mode(mode);
    
    if (result == 0) {
        g_boost_config.enabled = 1;
        g_boost_config.mode = mode;
    }
    
    pthread_mutex_unlock(&g_cpu_mutex);
    return result;
}

static int get_unique_capacities(int* capacities, int max_count) {
    int count = 0;
    for (int i = 0; i < MAX_CPU; i++) {
        if (g_cpu_manager.cpu_info[i].valid) {
            int found = 0;
            for (int j = 0; j < count; j++) {
                if (capacities[j] == g_cpu_manager.cpu_info[i].cpu_capacity) {
                    found = 1;
                    break;
                }
            }
            if (!found && count < max_count) {
                capacities[count++] = g_cpu_manager.cpu_info[i].cpu_capacity;
            }
        }
    }
    return count;
}

static void build_boost_entries(int capacity) {
    g_boost_config.last_adjust_time = get_time_ms();
}

static int l_setcpu(lua_State* L) {
    (void)L;
    return luaL_error(L, "setcpu requires root privileges");
}

static int l_getcpuinfo(lua_State* L) {
    pthread_mutex_lock(&g_cpu_mutex);
    update_cpu_info_cache();
    
    lua_newtable(L);
    
    lua_pushinteger(L, g_cpu_manager.cpu_count);
    lua_setfield(L, -2, "cpu_count");
    
    lua_pushinteger(L, g_cpu_manager.big_core_count);
    lua_setfield(L, -2, "big_core_count");
    
    lua_pushinteger(L, g_cpu_manager.little_core_count);
    lua_setfield(L, -2, "little_core_count");
    
    lua_pushinteger(L, g_cpu_manager.max_capacity);
    lua_setfield(L, -2, "max_capacity");
    
    lua_pushinteger(L, g_cpu_manager.min_capacity);
    lua_setfield(L, -2, "min_capacity");
    
    lua_newtable(L);
    for (int i = 0; i < MAX_CPU; i++) {
        if (g_cpu_manager.cpu_info[i].valid) {
            lua_newtable(L);
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].cpu_id);
            lua_setfield(L, -2, "cpu_id");
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].cpu_capacity);
            lua_setfield(L, -2, "cpu_capacity");
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].online);
            lua_setfield(L, -2, "online");
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].max_freq);
            lua_setfield(L, -2, "max_freq");
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].min_freq);
            lua_setfield(L, -2, "min_freq");
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].cur_freq);
            lua_setfield(L, -2, "cur_freq");
            
            lua_pushnumber(L, g_cpu_manager.cpu_info[i].bogomips);
            lua_setfield(L, -2, "bogomips");
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].implementer);
            lua_setfield(L, -2, "cpu_implementer");
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].architecture);
            lua_setfield(L, -2, "cpu_architecture");
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].variant);
            lua_setfield(L, -2, "cpu_variant");
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].part);
            lua_setfield(L, -2, "cpu_part");
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].revision);
            lua_setfield(L, -2, "cpu_revision");
            
            lua_pushinteger(L, g_cpu_manager.cpu_info[i].is_big_core);
            lua_setfield(L, -2, "is_big_core");
            
            lua_seti(L, -2, i + 1);
        }
    }
    lua_setfield(L, -2, "cpus");
    
    int capacities[16];
    int cap_count = get_unique_capacities(capacities, 16);
    lua_newtable(L);
    for (int i = 0; i < cap_count; i++) {
        lua_newtable(L);
        lua_pushinteger(L, capacities[i]);
        lua_setfield(L, -2, "capacity");
        
        build_boost_entries(capacities[i]);
        lua_newtable(L);
        for (int j = 0; j < MAX_CPU; j++) {
            if (g_cpu_manager.cpu_info[j].valid && 
                g_cpu_manager.cpu_info[j].cpu_capacity == capacities[i]) {
                lua_pushinteger(L, j);
                lua_seti(L, -2, lua_gettop(L) - 1);
            }
        }
        lua_setfield(L, -2, "cpus");
        
        lua_seti(L, -2, i + 1);
    }
    lua_setfield(L, -2, "capacities");
    
    lua_pushboolean(L, g_boost_config.enabled);
    lua_setfield(L, -2, "boost_enabled");
    
    lua_pushinteger(L, g_boost_config.mode);
    lua_setfield(L, -2, "current_mode");
    
    pthread_mutex_unlock(&g_cpu_mutex);
    
    return 1;
}

static int l_runlike(lua_State* L) {
    const char* mode_str = luaL_optlstring(L, 1, "", NULL);
    
    pthread_mutex_lock(&g_cpu_mutex);
    update_cpu_info_cache();
    
    performance_mode_t mode = PERFORMANCE_MODE_BALANCED;
    
    if (strcmp(mode_str, "LowPower") == 0) {
        mode = PERFORMANCE_MODE_LOW_POWER;
    } else if (strcmp(mode_str, "Balanced") == 0) {
        mode = PERFORMANCE_MODE_BALANCED;
    } else if (strcmp(mode_str, "Boost") == 0) {
        mode = PERFORMANCE_MODE_BOOST;
    } else if (strcmp(mode_str, "Turbo") == 0) {
        mode = PERFORMANCE_MODE_TURBO;
    } else if (strcmp(mode_str, "LowBoost") == 0) {
        mode = PERFORMANCE_MODE_LOW_BOOST;
    } else if (strcmp(mode_str, "MiddleBoost") == 0) {
        mode = PERFORMANCE_MODE_MIDDLE_BOOST;
    } else if (strcmp(mode_str, "HighBoost") == 0) {
        mode = PERFORMANCE_MODE_HIGH_BOOST;
    } else if (strcmp(mode_str, "DynamicBoost") == 0) {
        mode = PERFORMANCE_MODE_DYNAMIC;
    } else if (strcmp(mode_str, "Performance") == 0) {
        mode = PERFORMANCE_MODE_PERFORMANCE;
    } else {
        pthread_mutex_unlock(&g_cpu_mutex);
        return luaL_error(L, "invalid runlike mode: %s", mode_str);
    }
    
    int result = set_affinity_by_mode(mode);
    
    if (result == 0) {
        g_boost_config.enabled = 1;
        g_boost_config.mode = mode;
    }
    
    pthread_mutex_unlock(&g_cpu_mutex);
    
    if (result != 0) {
        return luaL_error(L, "failed to set CPU affinity (requires root): %s", strerror(errno));
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

static int l_setperformance(lua_State* L) {
    const char* mode_str = luaL_optlstring(L, 1, "", NULL);
    
    performance_mode_t mode = PERFORMANCE_MODE_BALANCED;
    
    if (strcmp(mode_str, "LowPower") == 0) {
        mode = PERFORMANCE_MODE_LOW_POWER;
    } else if (strcmp(mode_str, "Balanced") == 0) {
        mode = PERFORMANCE_MODE_BALANCED;
    } else if (strcmp(mode_str, "Boost") == 0) {
        mode = PERFORMANCE_MODE_BOOST;
    } else if (strcmp(mode_str, "Turbo") == 0) {
        mode = PERFORMANCE_MODE_TURBO;
    } else if (strcmp(mode_str, "LowBoost") == 0) {
        mode = PERFORMANCE_MODE_LOW_BOOST;
    } else if (strcmp(mode_str, "MiddleBoost") == 0) {
        mode = PERFORMANCE_MODE_MIDDLE_BOOST;
    } else if (strcmp(mode_str, "HighBoost") == 0) {
        mode = PERFORMANCE_MODE_HIGH_BOOST;
    } else if (strcmp(mode_str, "DynamicBoost") == 0) {
        mode = PERFORMANCE_MODE_DYNAMIC;
    } else if (strcmp(mode_str, "Performance") == 0) {
        mode = PERFORMANCE_MODE_PERFORMANCE;
    } else {
        return luaL_error(L, "invalid performance mode: %s", mode_str);
    }
    
    int result = lua_boost_set_performance_mode(mode);
    
    if (result != 0) {
        return luaL_error(L, "failed to set performance mode (requires root): %s", strerror(errno));
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

static int l_setfrequency(lua_State* L) {
    int n = lua_gettop(L);
    if (n < 2) {
        return luaL_error(L, "missing arguments: cpu_id and frequency");
    }
    
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2)) {
        return luaL_error(L, "cpu_id and frequency must be numbers");
    }
    
    int cpu_id = lua_tointeger(L, 1);
    long freq = lua_tointeger(L, 2);
    
    int result = lua_boost_set_cpu_frequency(cpu_id, freq);
    
    if (result != 0) {
        return luaL_error(L, "failed to set CPU frequency: %s", strerror(errno));
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

static int l_getfrequency(lua_State* L) {
    int cpu_id = luaL_optinteger(L, 1, -1);
    
    long freq;
    if (cpu_id >= 0) {
        freq = lua_boost_get_current_frequency(cpu_id);
    } else {
        cpu_id = sched_getcpu();
        if (cpu_id < 0) {
            freq = -1;
        } else {
            freq = lua_boost_get_current_frequency(cpu_id);
        }
    }
    
    lua_pushinteger(L, freq);
    return 1;
}

static int l_setcpuaffinity(lua_State* L) {
    int n = lua_gettop(L);
    if (n < 1) {
        return luaL_error(L, "missing cpu id arguments");
    }
    
    pthread_mutex_lock(&g_cpu_mutex);
    
    cpu_set_t mask;
    CPU_ZERO(&mask);
    
    for (int i = 1; i <= n && i <= MAX_CPU; i++) {
        if (!lua_isnumber(L, i)) {
            pthread_mutex_unlock(&g_cpu_mutex);
            return luaL_error(L, "All cpu-id arguments must be numbers");
        }
        int cpu_id = lua_tointeger(L, i);
        
        if (cpu_id < 0 || cpu_id >= MAX_CPU) {
            pthread_mutex_unlock(&g_cpu_mutex);
            return luaL_error(L, "invalid cpu id: %d", cpu_id);
        }
        
        if (g_cpu_manager.cpu_info[cpu_id].valid) {
            CPU_SET(cpu_id, &mask);
        }
    }
    
    pid_t pid = getpid();
    int result = sched_setaffinity(pid, sizeof(cpu_set_t), &mask);
    
    pthread_mutex_unlock(&g_cpu_mutex);
    
    if (result != 0) {
        return luaL_error(L, "failed to set CPU affinity: %s", strerror(errno));
    }
    
    lua_pushboolean(L, 1);
    return 1;
}

static int l_getcpucount(lua_State* L) {
    lua_pushinteger(L, lua_boost_get_cpu_count());
    return 1;
}

static int l_getbigcorecount(lua_State* L) {
    lua_pushinteger(L, lua_boost_get_big_core_count());
    return 1;
}

static int l_isbigcore(lua_State* L) {
    int cpu_id = luaL_checkinteger(L, 1);
    
    lua_pushboolean(L, lua_boost_is_big_core(cpu_id));
    return 1;
}

static int l_getmaxcapacity(lua_State* L) {
    lua_pushinteger(L, lua_boost_get_max_capacity());
    return 1;
}

static int l_getmincapacity(lua_State* L) {
    lua_pushinteger(L, lua_boost_get_min_capacity());
    return 1;
}

static int l_getcurrentcpu(lua_State* L) {
    int cpu_id = sched_getcpu();
    lua_pushinteger(L, cpu_id);
    return 1;
}

static int l_updatecache(lua_State* L) {
    pthread_mutex_lock(&g_cpu_mutex);
    update_cpu_info_cache();
    pthread_mutex_unlock(&g_cpu_mutex);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_getuptime(lua_State* L) {
    int64_t uptime = get_time_ms() - g_start_time_ms;
    lua_pushinteger(L, (lua_Integer)(uptime / 1000));
    return 1;
}

static int l_runtaskwithaffinity(lua_State* L) {
    int n = lua_gettop(L);
    if (n < 1) {
        return luaL_error(L, "missing function argument");
    }
    
    if (!lua_isfunction(L, 1)) {
        return luaL_error(L, "first argument must be a function");
    }
    
    int cpu_list[MAX_CPU];
    int cpu_count = 0;
    
    for (int i = 2; i <= n && cpu_count < MAX_CPU; i++) {
        if (lua_isnumber(L, i)) {
            cpu_list[cpu_count++] = lua_tointeger(L, i);
        }
    }
    
    cpu_set_t new_mask;
    CPU_ZERO(&new_mask);
    
    if (cpu_count > 0) {
        for (int i = 0; i < cpu_count; i++) {
            if (cpu_list[i] >= 0 && cpu_list[i] < MAX_CPU) {
                CPU_SET(cpu_list[i], &new_mask);
            }
        }
    } else {
        for (int i = 0; i < MAX_CPU; i++) {
            if (g_cpu_manager.cpu_info[i].valid) {
                CPU_SET(i, &new_mask);
            }
        }
    }
    
    cpu_set_t old_mask;
    CPU_ZERO(&old_mask);
    pid_t pid = getpid();
    
    if (sched_getaffinity(pid, sizeof(cpu_set_t), &old_mask) != 0) {
        return luaL_error(L, "failed to get current CPU affinity: %s", strerror(errno));
    }
    
    if (sched_setaffinity(pid, sizeof(cpu_set_t), &new_mask) != 0) {
        return luaL_error(L, "failed to set CPU affinity: %s", strerror(errno));
    }
    
    lua_pushvalue(L, 1);
    
    int argc = n - 1;
    if (argc > 0) {
        for (int i = 2; i <= n; i++) {
            lua_pushvalue(L, i);
        }
    }
    
    int status = lua_pcall(L, argc, LUA_MULTRET, 0);
    
    int result_count = lua_gettop(L);
    
    if (status != 0) {
        const char* error_msg = lua_tostring(L, -1);
        sched_setaffinity(pid, sizeof(cpu_set_t), &old_mask);
        return luaL_error(L, "function execution failed: %s", error_msg);
    }
    
    if (sched_setaffinity(pid, sizeof(cpu_set_t), &old_mask) != 0) {
        return luaL_error(L, "failed to restore CPU affinity: %s", strerror(errno));
    }
    
    return result_count;
}

static int l_runtaskwithmode(lua_State* L) {
    const char* mode_str;
    int n = lua_gettop(L);
    
    if (n < 2) {
        return luaL_error(L, "missing arguments: mode and function");
    }
    
    if (!lua_isfunction(L, 2)) {
        return luaL_error(L, "second argument must be a function");
    }
    
    mode_str = luaL_checkstring(L, 1);
    
    performance_mode_t mode = PERFORMANCE_MODE_BALANCED;
    
    if (strcmp(mode_str, "LowPower") == 0) {
        mode = PERFORMANCE_MODE_LOW_POWER;
    } else if (strcmp(mode_str, "Balanced") == 0) {
        mode = PERFORMANCE_MODE_BALANCED;
    } else if (strcmp(mode_str, "Boost") == 0) {
        mode = PERFORMANCE_MODE_BOOST;
    } else if (strcmp(mode_str, "Turbo") == 0) {
        mode = PERFORMANCE_MODE_TURBO;
    } else if (strcmp(mode_str, "LowBoost") == 0) {
        mode = PERFORMANCE_MODE_LOW_BOOST;
    } else if (strcmp(mode_str, "MiddleBoost") == 0) {
        mode = PERFORMANCE_MODE_MIDDLE_BOOST;
    } else if (strcmp(mode_str, "HighBoost") == 0) {
        mode = PERFORMANCE_MODE_HIGH_BOOST;
    } else if (strcmp(mode_str, "DynamicBoost") == 0) {
        mode = PERFORMANCE_MODE_DYNAMIC;
    } else if (strcmp(mode_str, "Performance") == 0) {
        mode = PERFORMANCE_MODE_PERFORMANCE;
    } else {
        return luaL_error(L, "invalid mode: %s", mode_str);
    }
    
    cpu_set_t new_mask;
    CPU_ZERO(&new_mask);
    
    int use_big_cores = 0;
    int use_little_cores = 0;
    int core_count = 0;
    
    switch (mode) {
        case PERFORMANCE_MODE_LOW_POWER:
            use_big_cores = 0;
            use_little_cores = 1;
            core_count = 1;
            break;
            
        case PERFORMANCE_MODE_LOW_BOOST:
            use_big_cores = 1;
            use_little_cores = 0;
            core_count = 1;
            break;
            
        case PERFORMANCE_MODE_BALANCED:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = 2;
            break;
            
        case PERFORMANCE_MODE_MIDDLE_BOOST:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = g_cpu_manager.big_core_count + 2;
            break;
            
        case PERFORMANCE_MODE_BOOST:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = g_cpu_manager.big_core_count + g_cpu_manager.little_core_count / 3;
            break;
            
        case PERFORMANCE_MODE_HIGH_BOOST:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = g_cpu_manager.big_core_count + g_cpu_manager.little_core_count * 2 / 3;
            break;
            
        case PERFORMANCE_MODE_TURBO:
            use_big_cores = 1;
            use_little_cores = 0;
            core_count = g_cpu_manager.big_core_count;
            break;
            
        case PERFORMANCE_MODE_PERFORMANCE:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = g_cpu_manager.big_core_count + g_cpu_manager.little_core_count;
            break;
            
        case PERFORMANCE_MODE_DYNAMIC:
            use_big_cores = 1;
            use_little_cores = 1;
            core_count = g_cpu_manager.big_core_count + g_cpu_manager.little_core_count;
            break;
            
        default:
            return luaL_error(L, "invalid mode: %s", mode_str);
    }
    
    int big_core_count = 0;
    int little_core_count = 0;
    
    if (use_big_cores) {
        for (int i = 0; i < MAX_CPU && big_core_count < g_cpu_manager.big_core_count && big_core_count < core_count; i++) {
            if (g_cpu_manager.cpu_info[i].valid && g_cpu_manager.cpu_info[i].is_big_core) {
                CPU_SET(i, &new_mask);
                big_core_count++;
            }
        }
    }
    
    if (use_little_cores && core_count > big_core_count) {
        int needed_little = core_count - big_core_count;
        for (int i = 0; i < MAX_CPU && little_core_count < needed_little; i++) {
            if (g_cpu_manager.cpu_info[i].valid && !g_cpu_manager.cpu_info[i].is_big_core) {
                CPU_SET(i, &new_mask);
                little_core_count++;
            }
        }
    }
    
    if (CPU_COUNT(&new_mask) == 0) {
        for (int i = 0; i < MAX_CPU && core_count > 0; i++) {
            if (g_cpu_manager.cpu_info[i].valid) {
                CPU_SET(i, &new_mask);
                core_count--;
            }
        }
    }
    
    cpu_set_t old_mask;
    CPU_ZERO(&old_mask);
    pid_t pid = getpid();
    
    if (sched_getaffinity(pid, sizeof(cpu_set_t), &old_mask) != 0) {
        return luaL_error(L, "failed to get current CPU affinity: %s", strerror(errno));
    }
    
    if (sched_setaffinity(pid, sizeof(cpu_set_t), &new_mask) != 0) {
        return luaL_error(L, "failed to set CPU affinity (requires root): %s", strerror(errno));
    }
    
    lua_pushvalue(L, 2);
    
    int argc = n - 2;
    if (argc > 0) {
        for (int i = 3; i <= n; i++) {
            lua_pushvalue(L, i);
        }
    }
    
    int status = lua_pcall(L, argc, LUA_MULTRET, 0);
    
    int result_count = lua_gettop(L);
    
    if (status != 0) {
        const char* error_msg = lua_tostring(L, -1);
        sched_setaffinity(pid, sizeof(cpu_set_t), &old_mask);
        return luaL_error(L, "function execution failed: %s", error_msg);
    }
    
    if (sched_setaffinity(pid, sizeof(cpu_set_t), &old_mask) != 0) {
        return luaL_error(L, "failed to restore CPU affinity: %s", strerror(errno));
    }
    
    return result_count;
}

static const luaL_Reg LuaBoost_funcs[] = {
    {"setcpu", l_setcpu},
    {"getcpuinfo", l_getcpuinfo},
    {"runlike", l_runlike},
    {"setperformance", l_setperformance},
    {"setfrequency", l_setfrequency},
    {"getfrequency", l_getfrequency},
    {"setcpuaffinity", l_setcpuaffinity},
    {"getcpucount", l_getcpucount},
    {"getbigcorecount", l_getbigcorecount},
    {"isbigcore", l_isbigcore},
    {"getmaxcapacity", l_getmaxcapacity},
    {"getmincapacity", l_getmincapacity},
    {"getcurrentcpu", l_getcurrentcpu},
    {"updatecache", l_updatecache},
    {"getuptime", l_getuptime},
    {"runtaskwithaffinity", l_runtaskwithaffinity},
    {"runtaskwithmode", l_runtaskwithmode},
    {NULL, NULL}
};

static const char* HELP_TEXT = 
"\n注意：部分功能需要 root 权限才能使用"
"\n"
"\nsetcpu:"
"\n    设置一个或多个CPU核心上线"
"\n    需要 root 权限"
"\n    参数: cpu_id1, cpu_id2, ..."
"\n    示例: LuaBoost.setcpu(0, 1, 2)"
"\n"
"\ngetcpuinfo:"
"\n    获取CPU信息，包括频率、容量等"
"\n    无需 root 权限"
"\n    返回: 包含cpu_count、cpus[]和capacities[]的表"
"\n"
"\nrunlike:"
"\n    根据模式设置CPU亲和性"
"\n    需要 root 权限"
"\n    模式: LowPower, Balanced, Boost, Turbo, LowBoost, MiddleBoost, HighBoost, DynamicBoost, Performance"
"\n    示例: LuaBoost.runlike('HighBoost')"
"\n"
"\nsetperformance:"
"\n    设置性能模式（推荐使用）"
"\n    需要 root 权限"
"\n    模式: LowPower, Balanced, Boost, Turbo, LowBoost, MiddleBoost, HighBoost, DynamicBoost, Performance"
"\n    示例: LuaBoost.setperformance('Performance')"
"\n"
"\nsetfrequency:"
"\n    设置指定CPU核心的频率"
"\n    需要 root 权限"
"\n    参数: cpu_id, frequency(in Hz)"
"\n    示例: LuaBoost.setfrequency(0, 1800000000)"
"\n"
"\ngetfrequency:"
"\n    获取CPU核心当前频率"
"\n    无需 root 权限"
"\n    参数: cpu_id (可选，默认当前核心)"
"\n    返回: 频率(Hz)"
"\n"
"\nsetcpuaffinity:"
"\n    设置进程CPU亲和性绑定"
"\n    无需 root 权限（仅影响当前进程）"
"\n    参数: cpu_id1, cpu_id2, ..."
"\n    示例: LuaBoost.setcpuaffinity(0, 1, 2, 3)"
"\n"
"\nruntaskwithaffinity:"
"\n    使用指定的CPU亲和性运行一个函数"
"\n    无需 root 权限（仅影响当前进程）"
"\n    参数: function, cpu_id1, cpu_id2, ..."
"\n    返回: 函数的返回值"
"\n    示例: "
"\n        local result = LuaBoost.runtaskwithaffinity(function()"
"\n            -- 这里的代码会在指定的CPU核心上运行"
"\n            return heavy_computation()"
"\n        end, 0, 1, 2, 3)"
"\n"
"\nruntaskwithmode:"
"\n    使用指定的性能模式运行一个函数"
"\n    需要 root 权限"
"\n    参数: mode, function, arg1, arg2, ..."
"\n    返回: 函数的返回值"
"\n    模式: LowPower, Balanced, Boost, Turbo, LowBoost, MiddleBoost, HighBoost, DynamicBoost, Performance"
"\n    示例: "
"\n        local result = LuaBoost.runtaskwithmode('Turbo', function(x, y)"
"\n            -- 这里的代码会在Turbo模式下运行（仅大核心）"
"\n            return x * y"
"\n        end, 10, 20)"
"\n"
"\ngetcpucount:"
"\n    获取CPU核心数量"
"\n    无需 root 权限"
"\n    返回: 整数"
"\n"
"\ngetbigcorecount:"
"\n    获取大核心数量"
"\n    无需 root 权限"
"\n    返回: 整数"
"\n"
"\nisbigcore:"
"\n    检查指定核心是否为大核心"
"\n    无需 root 权限"
"\n    参数: cpu_id"
"\n    返回: boolean"
"\n"
"\ngetmaxcapacity:"
"\n    获取最大CPU容量值"
"\n    无需 root 权限"
"\n    返回: 整数"
"\n"
"\ngetmincapacity:"
"\n    获取最小CPU容量值"
"\n    无需 root 权限"
"\n    返回: 整数"
"\n"
"\ngetcurrentcpu:"
"\n    获取当前执行的CPU核心"
"\n    无需 root 权限"
"\n    返回: cpu_id"
"\n"
"\nupdatecache:"
"\n    强制更新CPU信息缓存"
"\n    无需 root 权限"
"\n"
"\ngetuptime:"
"\n    获取模块运行时间（秒）"
"\n    无需 root 权限"
"\n    返回: 秒数";

static const char* AUTHOR_TEXT = "DifierLine";

int luaopen_LuaBoost(lua_State* L) {
    lua_boost_init();
    
    luaL_newlib(L, LuaBoost_funcs);
    
    lua_pushstring(L, HELP_TEXT);
    lua_setfield(L, -2, "__HELP");
    
    lua_pushstring(L, AUTHOR_TEXT);
    lua_setfield(L, -2, "__AUTHOR");
    
    lua_pushinteger(L, PERFORMANCE_MODE_LOW_POWER);
    lua_setfield(L, -2, "MODE_LOW_POWER");
    
    lua_pushinteger(L, PERFORMANCE_MODE_BALANCED);
    lua_setfield(L, -2, "MODE_BALANCED");
    
    lua_pushinteger(L, PERFORMANCE_MODE_BOOST);
    lua_setfield(L, -2, "MODE_BOOST");
    
    lua_pushinteger(L, PERFORMANCE_MODE_TURBO);
    lua_setfield(L, -2, "MODE_TURBO");
    
    lua_pushinteger(L, PERFORMANCE_MODE_LOW_BOOST);
    lua_setfield(L, -2, "MODE_LOW_BOOST");
    
    lua_pushinteger(L, PERFORMANCE_MODE_MIDDLE_BOOST);
    lua_setfield(L, -2, "MODE_MIDDLE_BOOST");
    
    lua_pushinteger(L, PERFORMANCE_MODE_HIGH_BOOST);
    lua_setfield(L, -2, "MODE_HIGH_BOOST");
    
    lua_pushinteger(L, PERFORMANCE_MODE_DYNAMIC);
    lua_setfield(L, -2, "MODE_DYNAMIC");
    
    lua_pushinteger(L, PERFORMANCE_MODE_PERFORMANCE);
    lua_setfield(L, -2, "MODE_PERFORMANCE");
    
    return 1;
}
