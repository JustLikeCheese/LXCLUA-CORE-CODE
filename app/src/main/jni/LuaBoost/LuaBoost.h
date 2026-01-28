#ifndef LUA_BOOST_H
#define LUA_BOOST_H

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdint.h>

#define LUA_BOOST_LIBNAME "LuaBoost"

#define MAX_CPU 16
#define MAX_PATH 128
#define CPU_CACHE_TIMEOUT_MS 5000

typedef enum {
    PERFORMANCE_MODE_LOW_POWER = 1,
    PERFORMANCE_MODE_BALANCED = 2,
    PERFORMANCE_MODE_BOOST = 3,
    PERFORMANCE_MODE_TURBO = 4,
    PERFORMANCE_MODE_LOW_BOOST = 5,
    PERFORMANCE_MODE_MIDDLE_BOOST = 6,
    PERFORMANCE_MODE_HIGH_BOOST = 7,
    PERFORMANCE_MODE_DYNAMIC = 8,
    PERFORMANCE_MODE_PERFORMANCE = 9
} performance_mode_t;

typedef struct {
    int cpu_id;
    int valid;
    int cpu_capacity;
    int online;
    long max_freq;
    long min_freq;
    long cur_freq;
    long target_freq;
    double bogomips;
    int implementer;
    int architecture;
    int variant;
    int part;
    int revision;
    int is_big_core;
    int thermal_state;
} cpu_info_t;

typedef struct {
    int type;
    int value;
} boost_entry_t;

typedef struct {
    cpu_info_t cpu_info[MAX_CPU];
    int cpu_count;
    int initialized;
    int64_t last_update_time;
    int unique_capacities[MAX_CPU];
    int capacity_count;
    int big_core_count;
    int little_core_count;
    int max_capacity;
    int min_capacity;
} cpu_manager_t;

typedef struct {
    int enabled;
    performance_mode_t mode;
    int auto_adjust;
    int64_t last_adjust_time;
} boost_config_t;

extern cpu_manager_t g_cpu_manager;
extern boost_config_t g_boost_config;

int luaopen_LuaBoost(lua_State* L);

void lua_boost_init(void);
void lua_boost_shutdown(void);
int lua_boost_set_performance_mode(performance_mode_t mode);
int lua_boost_get_cpu_count(void);
int lua_boost_get_big_core_count(void);
int lua_boost_set_cpu_affinity(int* cpu_list, int count);
int lua_boost_set_cpu_frequency(int cpu_id, long freq);
long lua_boost_get_current_frequency(int cpu_id);
int lua_boost_is_big_core(int cpu_id);
int lua_boost_get_max_capacity(void);
int lua_boost_get_min_capacity(void);

#endif
