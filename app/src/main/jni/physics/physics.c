/**
 * @file physics.c
 * @brief Lua物理模块实现 - 物理世界基本属性与计算
 * @author 物理模块
 * @version 1.1
 * @date 2024
 */

#include "physics.h"
#include <math.h>

#define VECTOR_METATABLE "PhysicsVector"
#define SCALAR_METATABLE "PhysicsScalar"
#define PHYS_OBJ_METATABLE "PhysicsObject"

/**
 * @brief 获取物理常量
 * @return 物理常量结构体
 */
PhysicalConstants get_physical_constants(void) {
    PhysicalConstants constants = {
        .speed_of_light = 299792458.0,
        .gravitational_constant = 6.67430e-11,
        .planck_constant = 6.62607015e-34,
        .reduced_planck_constant = 1.054571817e-34,
        .boltzmann_constant = 1.380649e-23,
        .avogadro_number = 6.02214076e23,
        .elementary_charge = 1.602176634e-19,
        .electron_mass = 9.1093837015e-31,
        .proton_mass = 1.67262192369e-27,
        .neutron_mass = 1.67492749804e-27,
        .earth_gravity = 9.80665,
        .gas_constant = 8.314462618,
        .standard_pressure = 101325.0,
        .standard_temperature = 273.15,
        .stefan_boltzmann_constant = 5.670374419e-5,
        .vacuum_permeability = 1.25663706212e-6,
        .vacuum_permittivity = 8.8541878128e-12,
        .bohr_magneton = 9.2740100783e-24,
        .bohr_radius = 5.29177210903e-11,
        .rydberg_constant = 10973731.568160,
        .faraday_constant = 96485.33212,
        .wien_displacement = 2.897771955e-3,
        .stephen_boltzmann = 5.670374419e-8
    };
    return constants;
}

/**
 * @brief 创建新的向量对象
 */
PhysicsVector* lua_new_vector(lua_State* L) {
    PhysicsVector* vec = (PhysicsVector*)lua_newuserdata(L, sizeof(PhysicsVector));
    luaL_getmetatable(L, VECTOR_METATABLE);
    lua_setmetatable(L, -2);
    
    vec->x = 0.0;
    vec->y = 0.0;
    vec->z = 0.0;
    
    return vec;
}

/**
 * @brief 检查参数是否为向量
 */
PhysicsVector* luaL_check_vector(lua_State* L, int index) {
    PhysicsVector* vec = (PhysicsVector*)luaL_checkudata(L, index, VECTOR_METATABLE);
    if (vec == NULL) {
        luaL_argerror(L, index, "expected PhysicsVector");
    }
    return vec;
}

/**
 * @brief 创建新的标量对象
 */
PhysicsScalar* lua_new_scalar(lua_State* L) {
    PhysicsScalar* scalar = (PhysicsScalar*)lua_newuserdata(L, sizeof(PhysicsScalar));
    luaL_getmetatable(L, SCALAR_METATABLE);
    lua_setmetatable(L, -2);
    
    scalar->value = 0.0;
    scalar->unit = "";
    
    return scalar;
}

/**
 * @brief 检查参数是否为标量
 */
PhysicsScalar* luaL_check_scalar(lua_State* L, int index) {
    PhysicsScalar* scalar = (PhysicsScalar*)luaL_checkudata(L, index, SCALAR_METATABLE);
    if (scalar == NULL) {
        luaL_argerror(L, index, "expected PhysicsScalar");
    }
    return scalar;
}

/**
 * @brief 创建新的物理对象
 */
PhysicsObject* lua_new_phys_obj(lua_State* L) {
    PhysicsObject* obj = (PhysicsObject*)lua_newuserdata(L, sizeof(PhysicsObject));
    luaL_getmetatable(L, PHYS_OBJ_METATABLE);
    lua_setmetatable(L, -2);
    
    obj->position.x = 0.0;
    obj->position.y = 0.0;
    obj->position.z = 0.0;
    obj->velocity.x = 0.0;
    obj->velocity.y = 0.0;
    obj->velocity.z = 0.0;
    obj->acceleration.x = 0.0;
    obj->acceleration.y = 0.0;
    obj->acceleration.z = 0.0;
    obj->mass = 0.0;
    obj->charge = 0.0;
    obj->moment_of_inertia = 0.0;
    obj->angular_velocity.x = 0.0;
    obj->angular_velocity.y = 0.0;
    obj->angular_velocity.z = 0.0;
    obj->angular_momentum.x = 0.0;
    obj->angular_momentum.y = 0.0;
    obj->angular_momentum.z = 0.0;
    
    return obj;
}

/**
 * @brief 检查参数是否为物理对象
 */
PhysicsObject* luaL_check_phys_obj(lua_State* L, int index) {
    PhysicsObject* obj = (PhysicsObject*)luaL_checkudata(L, index, PHYS_OBJ_METATABLE);
    if (obj == NULL) {
        luaL_argerror(L, index, "expected PhysicsObject");
    }
    return obj;
}

/**
 * @brief 释放物理对象内存
 */
void physics_object_free(PhysicsObject* obj) {
    (void)obj;
}

/**
 * @brief 向量加法
 */
PhysicsVector vector_add(PhysicsVector a, PhysicsVector b) {
    PhysicsVector result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

/**
 * @brief 向量减法
 */
PhysicsVector vector_sub(PhysicsVector a, PhysicsVector b) {
    PhysicsVector result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

/**
 * @brief 向量数乘
 */
PhysicsVector vector_scale(PhysicsVector v, double scalar) {
    PhysicsVector result;
    result.x = v.x * scalar;
    result.y = v.y * scalar;
    result.z = v.z * scalar;
    return result;
}

/**
 * @brief 向量点积
 */
double vector_dot(PhysicsVector a, PhysicsVector b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
 * @brief 向量叉积
 */
PhysicsVector vector_cross(PhysicsVector a, PhysicsVector b) {
    PhysicsVector result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

/**
 * @brief 向量模长
 */
double vector_magnitude(PhysicsVector v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * @brief 向量归一化
 */
PhysicsVector vector_normalize(PhysicsVector v) {
    PhysicsVector result;
    double mag = vector_magnitude(v);
    if (mag > 0) {
        result.x = v.x / mag;
        result.y = v.y / mag;
        result.z = v.z / mag;
    } else {
        result.x = 0.0;
        result.y = 0.0;
        result.z = 0.0;
    }
    return result;
}

/**
 * @brief 向量距离
 */
double vector_distance(PhysicsVector a, PhysicsVector b) {
    return vector_magnitude(vector_sub(a, b));
}

/**
 * @brief 向量角度
 */
double vector_angle(PhysicsVector a, PhysicsVector b) {
    double dot = vector_dot(a, b);
    double mag_a = vector_magnitude(a);
    double mag_b = vector_magnitude(b);
    if (mag_a > 0 && mag_b > 0) {
        return acos(dot / (mag_a * mag_b));
    }
    return 0.0;
}

/**
 * @brief 向量标量三重积 a·(b×c)
 */
double vector_scalar_triple(PhysicsVector a, PhysicsVector b, PhysicsVector c) {
    return vector_dot(a, vector_cross(b, c));
}

/**
 * @brief 向量线性插值
 */
PhysicsVector vector_lerp(PhysicsVector a, PhysicsVector b, double t) {
    PhysicsVector result;
    result.x = a.x + t * (b.x - a.x);
    result.y = a.y + t * (b.y - a.y);
    result.z = a.z + t * (b.z - a.z);
    return result;
}

/**
 * @brief 向量反射
 */
PhysicsVector vector_reflect(PhysicsVector v, PhysicsVector normal) {
    double d = 2.0 * vector_dot(v, normal);
    return vector_sub(v, vector_scale(normal, d));
}

/**
 * @brief 标量转角度
 */
double radians_to_degrees(double radians) {
    return radians * 180.0 / PHYSICS_PI;
}

/**
 * @brief 角度转弧度
 */
double degrees_to_radians(double degrees) {
    return degrees * PHYSICS_PI / 180.0;
}

/**
 * @brief 计算动能
 */
double calculate_kinetic_energy(double mass, double velocity) {
    return 0.5 * mass * velocity * velocity;
}

/**
 * @brief 计算重力势能
 */
double calculate_potential_energy(double mass, double height, double g) {
    return mass * g * height;
}

/**
 * @brief 计算动量（标量版本）
 */
double calculate_momentum(double mass, double velocity) {
    return mass * velocity;
}

/**
 * @brief 计算力
 */
double calculate_force(double mass, double acceleration) {
    return mass * acceleration;
}

/**
 * @brief 计算功
 */
double calculate_work(double force, double distance, double angle) {
    return force * distance * cos(angle);
}

/**
 * @brief 计算功率
 */
double calculate_power(double work, double time) {
    if (time <= 0) return 0.0;
    return work / time;
}

/**
 * @brief 计算压强
 */
double calculate_pressure(double force, double area) {
    if (area <= 0) return 0.0;
    return force / area;
}

/**
 * @brief 计算密度
 */
double calculate_density(double mass, double volume) {
    if (volume <= 0) return 0.0;
    return mass / volume;
}

/**
 * @brief 计算重力
 */
double calculate_gravity(double mass, double g) {
    return mass * g;
}

/**
 * @brief 计算胡克定律力
 */
double calculate_hooke_force(double k, double x) {
    return -k * x;
}

/**
 * @brief 计算万有引力
 */
double calculate_gravitational_force(double m1, double m2, double r, double G) {
    if (r <= 0) return 0.0;
    return G * m1 * m2 / (r * r);
}

/**
 * @brief 计算库仑力
 */
double calculate_coulomb_force(double q1, double q2, double r, double k) {
    if (r <= 0) return 0.0;
    return k * fabs(q1 * q2) / (r * r);
}

/**
 * @brief 计算冲量
 */
double calculate_impulse(double force, double time) {
    return force * time;
}

/**
 * @brief 计算碰撞速度
 */
double calculate_collision_velocity(double m1, double v1, double m2, double v2, int elastic) {
    if (elastic) {
        return ((m1 - m2) * v1 + 2 * m2 * v2) / (m1 + m2);
    } else {
        return (m1 * v1 + m2 * v2) / (m1 + m2);
    }
}

/**
 * @brief 计算转动动能
 */
double calculate_rotational_kinetic(double I, double omega) {
    return 0.5 * I * omega * omega;
}

/**
 * @brief 计算角动量
 */
double calculate_angular_momentum(double I, double omega) {
    return I * omega;
}

/**
 * @brief 计算力矩
 */
double calculate_torque(double r, double F, double angle) {
    return r * F * sin(angle);
}

/**
 * @brief 计算向心力
 */
double calculate_centripetal_force(double m, double v, double r) {
    if (r <= 0) return 0.0;
    return m * v * v / r;
}

/**
 * @brief 计算向心加速度
 */
double calculate_centripetal_acceleration(double v, double r) {
    if (r <= 0) return 0.0;
    return v * v / r;
}

/**
 * @brief 计算简谐运动位移
 */
double shm_displacement(double amplitude, double angular_frequency, double time, double phase) {
    return amplitude * sin(angular_frequency * time + phase);
}

/**
 * @brief 计算简谐运动速度
 */
double shm_velocity(double amplitude, double angular_frequency, double time, double phase) {
    return amplitude * angular_frequency * cos(angular_frequency * time + phase);
}

/**
 * @brief 计算简谐运动加速度
 */
double shm_acceleration(double amplitude, double angular_frequency, double time, double phase) {
    return -amplitude * angular_frequency * angular_frequency * sin(angular_frequency * time + phase);
}

/**
 * @brief 计算简谐运动周期
 */
double shm_period(double mass, double k) {
    if (k <= 0) return 0.0;
    return 2 * PHYSICS_PI * sqrt(mass / k);
}

/**
 * @brief 计算简谐运动频率
 */
double shm_frequency(double mass, double k) {
    double T = shm_period(mass, k);
    if (T <= 0) return 0.0;
    return 1.0 / T;
}

/**
 * @brief 计算简谐运动角频率
 */
double shm_angular_frequency(double mass, double k) {
    if (k <= 0) return 0.0;
    return sqrt(k / mass);
}

/**
 * @brief 计算波的频率
 */
double wave_frequency(double speed, double wavelength) {
    if (wavelength <= 0) return 0.0;
    return speed / wavelength;
}

/**
 * @brief 计算波的波长
 */
double wave_wavelength(double speed, double frequency) {
    if (frequency <= 0) return 0.0;
    return speed / frequency;
}

/**
 * @brief 计算波的周期
 */
double wave_period(double frequency) {
    if (frequency <= 0) return 0.0;
    return 1.0 / frequency;
}

/**
 * @brief 计算波的能量
 */
double wave_energy(double amplitude, double frequency, double mass_density) {
    return 2 * PHYSICS_PI * PHYSICS_PI * amplitude * amplitude * frequency * frequency * mass_density;
}

/**
 * @brief 计算波的强度
 */
double wave_intensity(double power, double area) {
    if (area <= 0) return 0.0;
    return power / area;
}

/**
 * @brief 计算理想气体压强
 */
double ideal_gas_pressure(double n, double R, double T, double V) {
    if (V <= 0) return 0.0;
    return n * R * T / V;
}

/**
 * @brief 计算理想气体状态方程
 */
double ideal_gas_law(double P, double V, double n, double R, double T, int solve_for_P) {
    if (solve_for_P) {
        if (V <= 0) return 0.0;
        return n * R * T / V;
    }
    return n * R * T;
}

/**
 * @brief 计算熵变
 */
double calculate_entropy_change(double Q, double T) {
    if (T <= 0) return 0.0;
    return Q / T;
}

/**
 * @brief 计算比热容
 */
double calculate_specific_heat(double Q, double m, double delta_T) {
    if (m <= 0 || delta_T == 0) return 0.0;
    return Q / (m * delta_T);
}

/**
 * @brief 计算热传导
 */
double calculate_heat_conduction(double k, double A, double dT, double d) {
    if (d <= 0) return 0.0;
    return k * A * dT / d;
}

/**
 * @brief 计算热容
 */
double calculate_heat_capacity(double Q, double delta_T) {
    if (delta_T == 0) return 0.0;
    return Q / delta_T;
}

/**
 * @brief 计算斯特藩-玻尔兹曼辐射功率
 */
double stefan_boltzmann_power(double sigma, double area, double T, double emissivity) {
    return sigma * area * emissivity * pow(T, 4);
}

/**
 * @brief 计算德布罗意波长
 */
double de_broglie_wavelength(double h, double momentum) {
    if (momentum <= 0) return 0.0;
    return h / momentum;
}

/**
 * @brief 计算光子能量
 */
double photon_energy(double h, double frequency) {
    return h * frequency;
}

/**
 * @brief 计算光子动量
 */
double photon_momentum(double h, double wavelength) {
    if (wavelength <= 0) return 0.0;
    return h / wavelength;
}

/**
 * @brief 计算相对论质量
 */
double relativistic_mass(double rest_mass, double velocity, double c) {
    double beta = velocity / c;
    if (beta >= 1.0) return INFINITY;
    return rest_mass / sqrt(1.0 - beta * beta);
}

/**
 * @brief 计算相对论能量（总能量）
 */
double relativistic_energy(double rest_mass, double velocity, double c) {
    double beta = velocity / c;
    if (beta >= 1.0) return INFINITY;
    double gamma = 1.0 / sqrt(1.0 - beta * beta);
    return gamma * rest_mass * c * c;
}

/**
 * @brief 计算相对论动量
 */
double relativistic_momentum(double rest_mass, double velocity, double c) {
    double beta = velocity / c;
    if (beta >= 1.0) return INFINITY;
    double gamma = 1.0 / sqrt(1.0 - beta * beta);
    return gamma * rest_mass * velocity;
}

/**
 * @brief 计算时间膨胀
 */
double time_dilation(double proper_time, double velocity, double c) {
    double beta = velocity / c;
    if (beta >= 1.0) return INFINITY;
    return proper_time / sqrt(1.0 - beta * beta);
}

/**
 * @brief 计算长度收缩
 */
double length_contraction(double proper_length, double velocity, double c) {
    double beta = velocity / c;
    if (beta >= 1.0) return 0.0;
    return proper_length * sqrt(1.0 - beta * beta);
}

/**
 * @brief 计算电场强度
 */
double calculate_electric_field(double k, double Q, double r) {
    if (r <= 0) return 0.0;
    return k * fabs(Q) / (r * r);
}

/**
 * @brief 计算电势
 */
double calculate_electric_potential(double k, double Q, double r) {
    if (r <= 0) return 0.0;
    return k * Q / r;
}

/**
 * @brief 计算电势能
 */
double calculate_electric_potential_energy(double k, double Q1, double Q2, double r) {
    if (r <= 0) return 0.0;
    return k * Q1 * Q2 / r;
}

/**
 * @brief 计算电容
 */
double calculate_capacitance(double Q, double V) {
    if (V == 0) return 0.0;
    return Q / V;
}

/**
 * @brief 计算欧姆定律
 */
double calculate_ohms_law(double V, double I, double R, int solve_for_V) {
    if (solve_for_V) {
        return I * R;
    }
    return V / R;
}

/**
 * @brief 计算电功率
 */
double calculate_electric_power(double V, double I, double R) {
    if (R > 0) {
        return V * V / R;
    }
    return V * I;
}

/**
 * @brief 计算电阻率
 */
double calculate_resistivity(double R, double L, double A) {
    if (A <= 0) return 0.0;
    return R * A / L;
}

/**
 * @brief 计算电导率
 */
double calculate_conductivity(double rho) {
    if (rho <= 0) return 0.0;
    return 1.0 / rho;
}

/**
 * @brief 计算折射率
 */
double calculate_refractive_index(double c, double v) {
    if (v <= 0) return 0.0;
    return c / v;
}

/**
 * @brief 计算斯涅尔定律
 */
double calculate_snells_law(double n1, double n2, double theta1) {
    if (n2 == 0) return 0.0;
    double sin_theta2 = (n1 / n2) * sin(theta1);
    if (sin_theta2 > 1.0) return PHYSICS_PI / 2.0;
    return asin(sin_theta2);
}

/**
 * @brief 计算临界角
 */
double calculate_critical_angle(double n1, double n2) {
    if (n2 >= n1) return 0.0;
    return asin(n2 / n1);
}

/**
 * @brief 计算透镜焦度
 */
double calculate_lens_power(double f) {
    if (f == 0) return 0.0;
    return 1.0 / f;
}

/**
 * @brief 计算薄透镜公式
 */
double calculate_thin_lens(double f, double do_di, int solve_for_di) {
    if (solve_for_di) {
        if (f == 0) return 0.0;
        double one_over_di = 1.0 / f - 1.0 / do_di;
        if (one_over_di == 0) return INFINITY;
        return 1.0 / one_over_di;
    }
    if (do_di == 0) return 0.0;
    return 1.0 / do_di;
}

/**
 * @brief 计算多普勒频移
 */
double calculate_doppler_shift(double v, double c, double f0, int source_moving_toward) {
    if (source_moving_toward) {
        return f0 * c / (c - v);
    }
    return f0 * c / (c + v);
}

/**
 * @brief 计算康普顿散射波长偏移
 */
double calculate_compton_shift(double theta, double h, double m, double c) {
    double lambda_compton = h / (m * c);
    return lambda_compton * (1.0 - cos(theta));
}

/**
 * @brief physics.new(x, y, z) - 创建向量
 */
static int l_physics_new_vector(lua_State* L) {
    double x = luaL_optnumber(L, 1, 0.0);
    double y = luaL_optnumber(L, 2, 0.0);
    double z = luaL_optnumber(L, 3, 0.0);
    
    PhysicsVector* vec = lua_new_vector(L);
    vec->x = x;
    vec->y = y;
    vec->z = z;
    
    return 1;
}

/**
 * @brief physics.new_scalar(value, unit) - 创建标量
 */
static int l_physics_new_scalar(lua_State* L) {
    double value = luaL_optnumber(L, 1, 0.0);
    const char* unit = luaL_optstring(L, 2, "");
    
    PhysicsScalar* scalar = lua_new_scalar(L);
    scalar->value = value;
    scalar->unit = unit;
    
    return 1;
}

/**
 * @brief physics.new_object() - 创建物理对象
 */
static int l_physics_new_object(lua_State* L) {
    PhysicsObject* obj = lua_new_phys_obj(L);
    
    int top = lua_gettop(L);
    if (top >= 1) {
        obj->mass = luaL_checknumber(L, 1);
    }
    if (top >= 2) {
        obj->charge = luaL_checknumber(L, 2);
    }
    
    return 1;
}

/**
 * @brief physics.vector(x, y, z) - 创建向量（简写）
 */
static int l_physics_vector(lua_State* L) {
    return l_physics_new_vector(L);
}

/**
 * @brief physics.scalar(value, unit) - 创建标量（简写）
 */
static int l_physics_scalar(lua_State* L) {
    return l_physics_new_scalar(L);
}

/**
 * @brief physics:add(v1, v2) - 向量加法
 */
static int l_physics_vector_add(lua_State* L) {
    PhysicsVector* a = luaL_check_vector(L, 1);
    PhysicsVector* b = luaL_check_vector(L, 2);
    
    PhysicsVector* result = lua_new_vector(L);
    result->x = a->x + b->x;
    result->y = a->y + b->y;
    result->z = a->z + b->z;
    
    return 1;
}

/**
 * @brief physics:sub(v1, v2) - 向量减法
 */
static int l_physics_vector_sub(lua_State* L) {
    PhysicsVector* a = luaL_check_vector(L, 1);
    PhysicsVector* b = luaL_check_vector(L, 2);
    
    PhysicsVector* result = lua_new_vector(L);
    result->x = a->x - b->x;
    result->y = a->y - b->y;
    result->z = a->z - b->z;
    
    return 1;
}

/**
 * @brief physics:mul(v, s) - 向量数乘
 */
static int l_physics_vector_scale(lua_State* L) {
    PhysicsVector* v = luaL_check_vector(L, 1);
    double scalar = luaL_checknumber(L, 2);
    
    PhysicsVector* result = lua_new_vector(L);
    result->x = v->x * scalar;
    result->y = v->y * scalar;
    result->z = v->z * scalar;
    
    return 1;
}

/**
 * @brief physics:dot(v1, v2) - 向量点积
 */
static int l_physics_vector_dot(lua_State* L) {
    PhysicsVector* a = luaL_check_vector(L, 1);
    PhysicsVector* b = luaL_check_vector(L, 2);
    
    double result = a->x * b->x + a->y * b->y + a->z * b->z;
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:cross(v1, v2) - 向量叉积
 */
static int l_physics_vector_cross(lua_State* L) {
    PhysicsVector* a = luaL_check_vector(L, 1);
    PhysicsVector* b = luaL_check_vector(L, 2);
    
    PhysicsVector* result = lua_new_vector(L);
    result->x = a->y * b->z - a->z * b->y;
    result->y = a->z * b->x - a->x * b->z;
    result->z = a->x * b->y - a->y * b->x;
    
    return 1;
}

/**
 * @brief physics:mag(v) - 向量模长
 */
static int l_physics_vector_magnitude(lua_State* L) {
    PhysicsVector* v = luaL_check_vector(L, 1);
    double result = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:normalize(v) - 向量归一化
 */
static int l_physics_vector_normalize(lua_State* L) {
    PhysicsVector* v = luaL_check_vector(L, 1);
    double mag = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    
    PhysicsVector* result = lua_new_vector(L);
    if (mag > 0) {
        result->x = v->x / mag;
        result->y = v->y / mag;
        result->z = v->z / mag;
    }
    
    return 1;
}

/**
 * @brief physics:distance(v1, v2) - 向量距离
 */
static int l_physics_vector_distance(lua_State* L) {
    PhysicsVector* a = luaL_check_vector(L, 1);
    PhysicsVector* b = luaL_check_vector(L, 2);
    
    double dx = a->x - b->x;
    double dy = a->y - b->y;
    double dz = a->z - b->z;
    double result = sqrt(dx * dx + dy * dy + dz * dz);
    
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:angle(v1, v2) - 向量夹角
 */
static int l_physics_vector_angle(lua_State* L) {
    PhysicsVector* a = luaL_check_vector(L, 1);
    PhysicsVector* b = luaL_check_vector(L, 2);
    
    double dot = a->x * b->x + a->y * b->y + a->z * b->z;
    double mag_a = sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
    double mag_b = sqrt(b->x * b->x + b->y * b->y + b->z * b->z);
    
    double result = 0.0;
    if (mag_a > 0 && mag_b > 0) {
        result = acos(dot / (mag_a * mag_b));
    }
    
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:lerp(v1, v2, t) - 线性插值
 */
static int l_physics_vector_lerp(lua_State* L) {
    PhysicsVector* a = luaL_check_vector(L, 1);
    PhysicsVector* b = luaL_check_vector(L, 2);
    double t = luaL_checknumber(L, 3);
    
    PhysicsVector* result = lua_new_vector(L);
    result->x = a->x + t * (b->x - a->x);
    result->y = a->y + t * (b->y - a->y);
    result->z = a->z + t * (b->z - a->z);
    
    return 1;
}

/**
 * @brief physics:reflect(v, normal) - 向量反射
 */
static int l_physics_vector_reflect(lua_State* L) {
    PhysicsVector* v = luaL_check_vector(L, 1);
    PhysicsVector* normal = luaL_check_vector(L, 2);
    
    double mag_n = sqrt(normal->x * normal->x + normal->y * normal->y + normal->z * normal->z);
    if (mag_n <= 0) {
        return luaL_error(L, "normal vector cannot be zero");
    }
    
    double nx = normal->x / mag_n;
    double ny = normal->y / mag_n;
    double nz = normal->z / mag_n;
    
    double d = 2.0 * (v->x * nx + v->y * ny + v->z * nz);
    
    PhysicsVector* result = lua_new_vector(L);
    result->x = v->x - d * nx;
    result->y = v->y - d * ny;
    result->z = v->z - d * nz;
    
    return 1;
}

/**
 * @brief physics.constants() - 获取物理常量
 */
static int l_physics_constants(lua_State* L) {
    PhysicalConstants c = get_physical_constants();
    
    lua_createtable(L, 0, 30);
    
    lua_pushstring(L, "speed_of_light");
    lua_pushnumber(L, c.speed_of_light);
    lua_settable(L, -3);
    
    lua_pushstring(L, "gravitational_constant");
    lua_pushnumber(L, c.gravitational_constant);
    lua_settable(L, -3);
    
    lua_pushstring(L, "planck_constant");
    lua_pushnumber(L, c.planck_constant);
    lua_settable(L, -3);
    
    lua_pushstring(L, "reduced_planck_constant");
    lua_pushnumber(L, c.reduced_planck_constant);
    lua_settable(L, -3);
    
    lua_pushstring(L, "boltzmann_constant");
    lua_pushnumber(L, c.boltzmann_constant);
    lua_settable(L, -3);
    
    lua_pushstring(L, "avogadro_number");
    lua_pushnumber(L, c.avogadro_number);
    lua_settable(L, -3);
    
    lua_pushstring(L, "elementary_charge");
    lua_pushnumber(L, c.elementary_charge);
    lua_settable(L, -3);
    
    lua_pushstring(L, "electron_mass");
    lua_pushnumber(L, c.electron_mass);
    lua_settable(L, -3);
    
    lua_pushstring(L, "proton_mass");
    lua_pushnumber(L, c.proton_mass);
    lua_settable(L, -3);
    
    lua_pushstring(L, "neutron_mass");
    lua_pushnumber(L, c.neutron_mass);
    lua_settable(L, -3);
    
    lua_pushstring(L, "earth_gravity");
    lua_pushnumber(L, c.earth_gravity);
    lua_settable(L, -3);
    
    lua_pushstring(L, "gas_constant");
    lua_pushnumber(L, c.gas_constant);
    lua_settable(L, -3);
    
    lua_pushstring(L, "standard_pressure");
    lua_pushnumber(L, c.standard_pressure);
    lua_settable(L, -3);
    
    lua_pushstring(L, "standard_temperature");
    lua_pushnumber(L, c.standard_temperature);
    lua_settable(L, -3);
    
    lua_pushstring(L, "stefan_boltzmann_constant");
    lua_pushnumber(L, c.stefan_boltzmann_constant);
    lua_settable(L, -3);
    
    lua_pushstring(L, "vacuum_permeability");
    lua_pushnumber(L, c.vacuum_permeability);
    lua_settable(L, -3);
    
    lua_pushstring(L, "vacuum_permittivity");
    lua_pushnumber(L, c.vacuum_permittivity);
    lua_settable(L, -3);
    
    lua_pushstring(L, "bohr_magneton");
    lua_pushnumber(L, c.bohr_magneton);
    lua_settable(L, -3);
    
    lua_pushstring(L, "bohr_radius");
    lua_pushnumber(L, c.bohr_radius);
    lua_settable(L, -3);
    
    lua_pushstring(L, "rydberg_constant");
    lua_pushnumber(L, c.rydberg_constant);
    lua_settable(L, -3);
    
    lua_pushstring(L, "faraday_constant");
    lua_pushnumber(L, c.faraday_constant);
    lua_settable(L, -3);
    
    lua_pushstring(L, "wien_displacement");
    lua_pushnumber(L, c.wien_displacement);
    lua_settable(L, -3);
    
    lua_pushstring(L, "pi");
    lua_pushnumber(L, PHYSICS_PI);
    lua_settable(L, -3);
    
    lua_pushstring(L, "e");
    lua_pushnumber(L, PHYSICS_E);
    lua_settable(L, -3);
    
    return 1;
}

/**
 * @brief physics:impulse(F, t) - 计算冲量
 */
static int l_physics_impulse(lua_State* L) {
    double force = luaL_checknumber(L, 1);
    double time = luaL_checknumber(L, 2);
    lua_pushnumber(L, force * time);
    return 1;
}

/**
 * @brief physics:collision(m1, v1, m2, v2, elastic) - 计算碰撞
 */
static int l_physics_collision(lua_State* L) {
    double m1 = luaL_checknumber(L, 1);
    double v1 = luaL_checknumber(L, 2);
    double m2 = luaL_checknumber(L, 3);
    double v2 = luaL_checknumber(L, 4);
    int elastic = lua_isboolean(L, 5) ? lua_toboolean(L, 5) : 1;
    
    double result;
    if (elastic) {
        result = ((m1 - m2) * v1 + 2 * m2 * v2) / (m1 + m2);
    } else {
        result = (m1 * v1 + m2 * v2) / (m1 + m2);
    }
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:rotational_kinetic(I, omega) - 计算转动动能
 */
static int l_physics_rotational_kinetic(lua_State* L) {
    double I = luaL_checknumber(L, 1);
    double omega = luaL_checknumber(L, 2);
    lua_pushnumber(L, 0.5 * I * omega * omega);
    return 1;
}

/**
 * @brief physics:angular_momentum(I, omega) - 计算角动量
 */
static int l_physics_angular_momentum(lua_State* L) {
    double I = luaL_checknumber(L, 1);
    double omega = luaL_checknumber(L, 2);
    lua_pushnumber(L, I * omega);
    return 1;
}

/**
 * @brief physics:torque(r, F, angle) - 计算力矩
 */
static int l_physics_torque(lua_State* L) {
    double r = luaL_checknumber(L, 1);
    double F = luaL_checknumber(L, 2);
    double angle = luaL_optnumber(L, 3, PHYSICS_PI / 2.0);
    lua_pushnumber(L, r * F * sin(angle));
    return 1;
}

/**
 * @brief physics:centripetal_force(m, v, r) - 计算向心力
 */
static int l_physics_centripetal_force(lua_State* L) {
    double m = luaL_checknumber(L, 1);
    double v = luaL_checknumber(L, 2);
    double r = luaL_checknumber(L, 3);
    if (r <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, m * v * v / r);
    }
    return 1;
}

/**
 * @brief physics:centripetal_acceleration(v, r) - 计算向心加速度
 */
static int l_physics_centripetal_acceleration(lua_State* L) {
    double v = luaL_checknumber(L, 1);
    double r = luaL_checknumber(L, 2);
    if (r <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, v * v / r);
    }
    return 1;
}

/**
 * @brief physics:shm_period(m, k) - 计算简谐运动周期
 */
static int l_physics_shm_period(lua_State* L) {
    double m = luaL_checknumber(L, 1);
    double k = luaL_checknumber(L, 2);
    if (k <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, 2 * PHYSICS_PI * sqrt(m / k));
    }
    return 1;
}

/**
 * @brief physics:shm_frequency(m, k) - 计算简谐运动频率
 */
static int l_physics_shm_frequency(lua_State* L) {
    double m = luaL_checknumber(L, 1);
    double k = luaL_checknumber(L, 2);
    if (k <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, sqrt(k / m) / (2 * PHYSICS_PI));
    }
    return 1;
}

/**
 * @brief physics:wave_energy(A, f, rho) - 计算波的能量
 */
static int l_physics_wave_energy(lua_State* L) {
    double amplitude = luaL_checknumber(L, 1);
    double frequency = luaL_checknumber(L, 2);
    double rho = luaL_checknumber(L, 3);
    lua_pushnumber(L, 2 * PHYSICS_PI * PHYSICS_PI * amplitude * amplitude * frequency * frequency * rho);
    return 1;
}

/**
 * @brief physics:wave_intensity(P, A) - 计算波的强度
 */
static int l_physics_wave_intensity(lua_State* L) {
    double power = luaL_checknumber(L, 1);
    double area = luaL_checknumber(L, 2);
    if (area <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, power / area);
    }
    return 1;
}

/**
 * @brief physics:entropy_change(Q, T) - 计算熵变
 */
static int l_physics_entropy_change(lua_State* L) {
    double Q = luaL_checknumber(L, 1);
    double T = luaL_checknumber(L, 2);
    if (T <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, Q / T);
    }
    return 1;
}

/**
 * @brief physics:specific_heat(Q, m, dT) - 计算比热容
 */
static int l_physics_specific_heat(lua_State* L) {
    double Q = luaL_checknumber(L, 1);
    double m = luaL_checknumber(L, 2);
    double delta_T = luaL_checknumber(L, 3);
    if (m <= 0 || delta_T == 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, Q / (m * delta_T));
    }
    return 1;
}

/**
 * @brief physics:heat_conduction(k, A, dT, d) - 计算热传导
 */
static int l_physics_heat_conduction(lua_State* L) {
    double k = luaL_checknumber(L, 1);
    double A = luaL_checknumber(L, 2);
    double dT = luaL_checknumber(L, 3);
    double d = luaL_checknumber(L, 4);
    if (d <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, k * A * dT / d);
    }
    return 1;
}

/**
 * @brief physics:relativistic_momentum(m0, v, c) - 计算相对论动量
 */
static int l_physics_relativistic_momentum(lua_State* L) {
    double rest_mass = luaL_checknumber(L, 1);
    double velocity = luaL_checknumber(L, 2);
    double c = luaL_optnumber(L, 3, 299792458.0);
    
    double beta = velocity / c;
    if (beta >= 1.0) {
        lua_pushnumber(L, INFINITY);
    } else {
        double gamma = 1.0 / sqrt(1.0 - beta * beta);
        lua_pushnumber(L, gamma * rest_mass * velocity);
    }
    return 1;
}

/**
 * @brief physics:time_dilation(t, v, c) - 计算时间膨胀
 */
static int l_physics_time_dilation(lua_State* L) {
    double proper_time = luaL_checknumber(L, 1);
    double velocity = luaL_checknumber(L, 2);
    double c = luaL_optnumber(L, 3, 299792458.0);
    
    double beta = velocity / c;
    if (beta >= 1.0) {
        lua_pushnumber(L, INFINITY);
    } else {
        lua_pushnumber(L, proper_time / sqrt(1.0 - beta * beta));
    }
    return 1;
}

/**
 * @brief physics:length_contraction(L, v, c) - 计算长度收缩
 */
static int l_physics_length_contraction(lua_State* L) {
    double proper_length = luaL_checknumber(L, 1);
    double velocity = luaL_checknumber(L, 2);
    double c = luaL_optnumber(L, 3, 299792458.0);
    
    double beta = velocity / c;
    if (beta >= 1.0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, proper_length * sqrt(1.0 - beta * beta));
    }
    return 1;
}

/**
 * @brief physics:electric_field(k, Q, r) - 计算电场强度
 */
static int l_physics_electric_field(lua_State* L) {
    double k = luaL_optnumber(L, 1, 8.9875517923e9);
    double Q = luaL_checknumber(L, 2);
    double r = luaL_checknumber(L, 3);
    if (r <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, k * fabs(Q) / (r * r));
    }
    return 1;
}

/**
 * @brief physics:electric_potential(k, Q, r) - 计算电势
 */
static int l_physics_electric_potential(lua_State* L) {
    double k = luaL_optnumber(L, 1, 8.9875517923e9);
    double Q = luaL_checknumber(L, 2);
    double r = luaL_checknumber(L, 3);
    if (r <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, k * Q / r);
    }
    return 1;
}

/**
 * @brief physics:electric_potential_energy(k, Q1, Q2, r) - 计算电势能
 */
static int l_physics_electric_potential_energy(lua_State* L) {
    double k = luaL_optnumber(L, 1, 8.9875517923e9);
    double Q1 = luaL_checknumber(L, 2);
    double Q2 = luaL_checknumber(L, 3);
    double r = luaL_checknumber(L, 4);
    if (r <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, k * Q1 * Q2 / r);
    }
    return 1;
}

/**
 * @brief physics:capacitance(Q, V) - 计算电容
 */
static int l_physics_capacitance(lua_State* L) {
    double Q = luaL_checknumber(L, 1);
    double V = luaL_checknumber(L, 2);
    if (V == 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, Q / V);
    }
    return 1;
}

/**
 * @brief physics:ohms_law(V, I, R) - 欧姆定律
 */
static int l_physics_ohms_law(lua_State* L) {
    double V = luaL_checknumber(L, 1);
    double I = luaL_checknumber(L, 2);
    double R = luaL_optnumber(L, 3, 0.0);
    if (R > 0) {
        lua_pushnumber(L, I * R);
    } else {
        lua_pushnumber(L, V / I);
    }
    return 1;
}

/**
 * @brief physics:electric_power(V, I, R) - 计算电功率
 */
static int l_physics_electric_power(lua_State* L) {
    double V = luaL_checknumber(L, 1);
    double I = luaL_checknumber(L, 2);
    double R = luaL_optnumber(L, 3, 0.0);
    if (R > 0) {
        lua_pushnumber(L, V * V / R);
    } else {
        lua_pushnumber(L, V * I);
    }
    return 1;
}

/**
 * @brief physics:resistivity(R, L, A) - 计算电阻率
 */
static int l_physics_resistivity(lua_State* L) {
    double R = luaL_checknumber(L, 1);
    double length = luaL_checknumber(L, 2);
    double A = luaL_checknumber(L, 3);
    if (A <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, R * A / length);
    }
    return 1;
}

/**
 * @brief physics:refractive_index(c, v) - 计算折射率
 */
static int l_physics_refractive_index(lua_State* L) {
    double c = luaL_checknumber(L, 1);
    double v = luaL_checknumber(L, 2);
    if (v <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, c / v);
    }
    return 1;
}

/**
 * @brief physics:snells_law(n1, n2, theta1) - 斯涅尔定律
 */
static int l_physics_snells_law(lua_State* L) {
    double n1 = luaL_checknumber(L, 1);
    double n2 = luaL_checknumber(L, 2);
    double theta1 = luaL_checknumber(L, 3);
    if (n2 == 0) {
        lua_pushnumber(L, 0.0);
    } else {
        double sin_theta2 = (n1 / n2) * sin(theta1);
        if (sin_theta2 > 1.0) {
            lua_pushnumber(L, PHYSICS_PI / 2.0);
        } else {
            lua_pushnumber(L, asin(sin_theta2));
        }
    }
    return 1;
}

/**
 * @brief physics:critical_angle(n1, n2) - 计算临界角
 */
static int l_physics_critical_angle(lua_State* L) {
    double n1 = luaL_checknumber(L, 1);
    double n2 = luaL_checknumber(L, 2);
    if (n2 >= n1) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, asin(n2 / n1));
    }
    return 1;
}

/**
 * @brief physics:lens_power(f) - 计算透镜焦度
 */
static int l_physics_lens_power(lua_State* L) {
    double f = luaL_checknumber(L, 1);
    if (f == 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, 1.0 / f);
    }
    return 1;
}

/**
 * @brief physics:doppler_shift(v, c, f0, toward) - 多普勒频移
 */
static int l_physics_doppler_shift(lua_State* L) {
    double v = luaL_checknumber(L, 1);
    double c = luaL_optnumber(L, 2, 299792458.0);
    double f0 = luaL_checknumber(L, 3);
    int toward = lua_isboolean(L, 4) ? lua_toboolean(L, 4) : 1;
    
    if (toward) {
        lua_pushnumber(L, f0 * c / (c - v));
    } else {
        lua_pushnumber(L, f0 * c / (c + v));
    }
    return 1;
}

/**
 * @brief physics:compton_shift(theta, h, m, c) - 康普顿散射
 */
static int l_physics_compton_shift(lua_State* L) {
    double theta = luaL_checknumber(L, 1);
    double h = luaL_optnumber(L, 2, 6.62607015e-34);
    double m = luaL_optnumber(L, 3, 9.1093837015e-31);
    double c = luaL_optnumber(L, 4, 299792458.0);
    
    double lambda_compton = h / (m * c);
    lua_pushnumber(L, lambda_compton * (1.0 - cos(theta)));
    return 1;
}

/**
 * @brief physics:kinetic_energy(m, v) - 计算动能
 */
static int l_physics_kinetic_energy(lua_State* L) {
    double mass = luaL_checknumber(L, 1);
    double velocity = luaL_checknumber(L, 2);
    double result = 0.5 * mass * velocity * velocity;
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:potential_energy(m, h, g) - 计算重力势能
 */
static int l_physics_potential_energy(lua_State* L) {
    double mass = luaL_checknumber(L, 1);
    double height = luaL_checknumber(L, 2);
    double g = luaL_optnumber(L, 3, 9.80665);
    double result = mass * g * height;
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:momentum(m, v) - 计算动量
 */
static int l_physics_momentum(lua_State* L) {
    double mass = luaL_checknumber(L, 1);
    double velocity = luaL_checknumber(L, 2);
    double result = mass * velocity;
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:force(m, a) - 计算力
 */
static int l_physics_force(lua_State* L) {
    double mass = luaL_checknumber(L, 1);
    double acceleration = luaL_checknumber(L, 2);
    double result = mass * acceleration;
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:work(F, d, angle) - 计算功
 */
static int l_physics_work(lua_State* L) {
    double force = luaL_checknumber(L, 1);
    double distance = luaL_checknumber(L, 2);
    double angle = luaL_optnumber(L, 3, 0.0);
    double result = force * distance * cos(angle);
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:power(w, t) - 计算功率
 */
static int l_physics_power(lua_State* L) {
    double work = luaL_checknumber(L, 1);
    double time = luaL_checknumber(L, 2);
    if (time <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, work / time);
    }
    return 1;
}

/**
 * @brief physics:pressure(F, A) - 计算压强
 */
static int l_physics_pressure(lua_State* L) {
    double force = luaL_checknumber(L, 1);
    double area = luaL_checknumber(L, 2);
    if (area <= 0) {
        lua_pushnumber(L, 0.0);
    } else {
        lua_pushnumber(L, force / area);
    }
    return 1;
}

/**
 * @brief physics:gravity(m, g) - 计算重力
 */
static int l_physics_gravity(lua_State* L) {
    double mass = luaL_checknumber(L, 1);
    double g = luaL_optnumber(L, 2, 9.80665);
    lua_pushnumber(L, mass * g);
    return 1;
}

/**
 * @brief physics:hooke(k, x) - 计算胡克定律弹力
 */
static int l_physics_hooke(lua_State* L) {
    double k = luaL_checknumber(L, 1);
    double x = luaL_checknumber(L, 2);
    lua_pushnumber(L, -k * x);
    return 1;
}

/**
 * @brief physics:gravitational_force(m1, m2, r) - 计算万有引力
 */
static int l_physics_gravitational_force(lua_State* L) {
    double m1 = luaL_checknumber(L, 1);
    double m2 = luaL_checknumber(L, 2);
    double r = luaL_checknumber(L, 3);
    double G = 6.67430e-11;
    
    double result = 0.0;
    if (r > 0) {
        result = G * m1 * m2 / (r * r);
    }
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:coulomb_force(q1, q2, r) - 计算库仑力
 */
static int l_physics_coulomb_force(lua_State* L) {
    double q1 = luaL_checknumber(L, 1);
    double q2 = luaL_checknumber(L, 2);
    double r = luaL_checknumber(L, 3);
    double k = 8.9875517923e9;
    
    double result = 0.0;
    if (r > 0) {
        result = k * fabs(q1 * q2) / (r * r);
    }
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:shm_displacement(A, w, t, phi) - 简谐运动位移
 */
static int l_physics_shm_displacement(lua_State* L) {
    double amplitude = luaL_checknumber(L, 1);
    double w = luaL_checknumber(L, 2);
    double time = luaL_checknumber(L, 3);
    double phase = luaL_optnumber(L, 4, 0.0);
    double result = amplitude * sin(w * time + phase);
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:shm_velocity(A, w, t, phi) - 简谐运动速度
 */
static int l_physics_shm_velocity(lua_State* L) {
    double amplitude = luaL_checknumber(L, 1);
    double w = luaL_checknumber(L, 2);
    double time = luaL_checknumber(L, 3);
    double phase = luaL_optnumber(L, 4, 0.0);
    double result = amplitude * w * cos(w * time + phase);
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:shm_acceleration(A, w, t, phi) - 简谐运动加速度
 */
static int l_physics_shm_acceleration(lua_State* L) {
    double amplitude = luaL_checknumber(L, 1);
    double w = luaL_checknumber(L, 2);
    double time = luaL_checknumber(L, 3);
    double phase = luaL_optnumber(L, 4, 0.0);
    double result = -amplitude * w * w * sin(w * time + phase);
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:wave_frequency(v, lambda) - 计算波频率
 */
static int l_physics_wave_frequency(lua_State* L) {
    double speed = luaL_checknumber(L, 1);
    double wavelength = luaL_checknumber(L, 2);
    double result = 0.0;
    if (wavelength > 0) {
        result = speed / wavelength;
    }
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:wave_wavelength(v, f) - 计算波长
 */
static int l_physics_wave_wavelength(lua_State* L) {
    double speed = luaL_checknumber(L, 1);
    double frequency = luaL_checknumber(L, 2);
    double result = 0.0;
    if (frequency > 0) {
        result = speed / frequency;
    }
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:wave_period(f) - 计算波周期
 */
static int l_physics_wave_period(lua_State* L) {
    double frequency = luaL_checknumber(L, 1);
    double result = 0.0;
    if (frequency > 0) {
        result = 1.0 / frequency;
    }
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:ideal_gas_law(n, R, T, V) - 理想气体状态方程
 */
static int l_physics_ideal_gas_law(lua_State* L) {
    double n = luaL_checknumber(L, 1);
    double R = luaL_optnumber(L, 2, 8.314462618);
    double T = luaL_checknumber(L, 3);
    double V = luaL_checknumber(L, 4);
    
    double result = 0.0;
    if (V > 0) {
        result = n * R * T / V;
    }
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:relativistic_mass(m0, v, c) - 计算相对论质量
 */
static int l_physics_relativistic_mass(lua_State* L) {
    double rest_mass = luaL_checknumber(L, 1);
    double velocity = luaL_checknumber(L, 2);
    double c = luaL_optnumber(L, 3, 299792458.0);
    
    double beta = velocity / c;
    double result = 0.0;
    if (beta < 1.0) {
        result = rest_mass / sqrt(1.0 - beta * beta);
    } else {
        result = INFINITY;
    }
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:relativistic_energy(m0, v, c) - 计算相对论总能量
 */
static int l_physics_relativistic_energy(lua_State* L) {
    double rest_mass = luaL_checknumber(L, 1);
    double velocity = luaL_checknumber(L, 2);
    double c = luaL_optnumber(L, 3, 299792458.0);
    
    double beta = velocity / c;
    double result = 0.0;
    if (beta < 1.0) {
        double gamma = 1.0 / sqrt(1.0 - beta * beta);
        result = gamma * rest_mass * c * c;
    } else {
        result = INFINITY;
    }
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:photon_energy(f, h) - 计算光子能量
 */
static int l_physics_photon_energy(lua_State* L) {
    double frequency = luaL_checknumber(L, 1);
    double h = luaL_optnumber(L, 2, 6.62607015e-34);
    lua_pushnumber(L, h * frequency);
    return 1;
}

/**
 * @brief physics:de_broglie(momentum, h) - 计算德布罗意波长
 */
static int l_physics_de_broglie(lua_State* L) {
    double momentum = luaL_checknumber(L, 1);
    double h = luaL_optnumber(L, 2, 6.62607015e-34);
    double result = 0.0;
    if (momentum > 0) {
        result = h / momentum;
    }
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief physics:to_degrees(rad) - 弧度转角度
 */
static int l_physics_to_degrees(lua_State* L) {
    double radians = luaL_checknumber(L, 1);
    lua_pushnumber(L, radians * 180.0 / PHYSICS_PI);
    return 1;
}

/**
 * @brief physics:to_radians(deg) - 角度转弧度
 */
static int l_physics_to_radians(lua_State* L) {
    double degrees = luaL_checknumber(L, 1);
    lua_pushnumber(L, degrees * PHYSICS_PI / 180.0);
    return 1;
}

/**
 * @brief 向量元方法：gc
 */
static int l_vector_gc(lua_State* L) {
    (void)L;
    return 0;
}

/**
 * @brief 向量元方法：tostring
 */
static int l_vector_tostring(lua_State* L) {
    PhysicsVector* v = luaL_check_vector(L, 1);
    lua_pushfstring(L, "Vector(%.6g, %.6g, %.6g)", v->x, v->y, v->z);
    return 1;
}

/**
 * @brief 向量元方法：获取x分量
 */
static int l_vector_get_x(lua_State* L) {
    PhysicsVector* v = luaL_check_vector(L, 1);
    lua_pushnumber(L, v->x);
    return 1;
}

/**
 * @brief 向量元方法：获取y分量
 */
static int l_vector_get_y(lua_State* L) {
    PhysicsVector* v = luaL_check_vector(L, 1);
    lua_pushnumber(L, v->y);
    return 1;
}

/**
 * @brief 向量元方法：获取z分量
 */
static int l_vector_get_z(lua_State* L) {
    PhysicsVector* v = luaL_check_vector(L, 1);
    lua_pushnumber(L, v->z);
    return 1;
}

/**
 * @brief 向量元方法：设置x分量
 */
static int l_vector_set_x(lua_State* L) {
    PhysicsVector* v = luaL_check_vector(L, 1);
    v->x = luaL_checknumber(L, 2);
    return 0;
}

/**
 * @brief 向量元方法：设置y分量
 */
static int l_vector_set_y(lua_State* L) {
    PhysicsVector* v = luaL_check_vector(L, 1);
    v->y = luaL_checknumber(L, 2);
    return 0;
}

/**
 * @brief 向量元方法：设置z分量
 */
static int l_vector_set_z(lua_State* L) {
    PhysicsVector* v = luaL_check_vector(L, 1);
    v->z = luaL_checknumber(L, 2);
    return 0;
}

/**
 * @brief 标量元方法：gc
 */
static int l_scalar_gc(lua_State* L) {
    (void)L;
    return 0;
}

/**
 * @brief 标量元方法：tostring
 */
static int l_scalar_tostring(lua_State* L) {
    PhysicsScalar* s = luaL_check_scalar(L, 1);
    lua_pushfstring(L, "Scalar(%.6g %s)", s->value, s->unit);
    return 1;
}

/**
 * @brief 标量元方法：获取值
 */
static int l_scalar_get_value(lua_State* L) {
    PhysicsScalar* s = luaL_check_scalar(L, 1);
    lua_pushnumber(L, s->value);
    return 1;
}

/**
 * @brief 标量元方法：获取单位
 */
static int l_scalar_get_unit(lua_State* L) {
    PhysicsScalar* s = luaL_check_scalar(L, 1);
    lua_pushstring(L, s->unit);
    return 1;
}

/**
 * @brief 物理对象元方法：gc
 */
static int l_phys_obj_gc(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    physics_object_free(obj);
    return 0;
}

/**
 * @brief 物理对象元方法：tostring
 */
static int l_phys_obj_tostring(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    lua_pushfstring(L, "PhysicsObject(mass=%.6g, charge=%.6g)", obj->mass, obj->charge);
    return 1;
}

/**
 * @brief 物理对象：设置位置
 */
static int l_phys_obj_set_position(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    if (lua_gettop(L) >= 4) {
        obj->position.x = luaL_checknumber(L, 2);
        obj->position.y = luaL_checknumber(L, 3);
        obj->position.z = luaL_checknumber(L, 4);
    } else if (lua_gettop(L) >= 2 && lua_istable(L, 2)) {
        lua_getfield(L, 2, "x");
        obj->position.x = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
        lua_getfield(L, 2, "y");
        obj->position.y = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
        lua_getfield(L, 2, "z");
        obj->position.z = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
    }
    return 0;
}

/**
 * @brief 物理对象：设置速度
 */
static int l_phys_obj_set_velocity(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    if (lua_gettop(L) >= 4) {
        obj->velocity.x = luaL_checknumber(L, 2);
        obj->velocity.y = luaL_checknumber(L, 3);
        obj->velocity.z = luaL_checknumber(L, 4);
    } else if (lua_gettop(L) >= 2 && lua_istable(L, 2)) {
        lua_getfield(L, 2, "x");
        obj->velocity.x = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
        lua_getfield(L, 2, "y");
        obj->velocity.y = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
        lua_getfield(L, 2, "z");
        obj->velocity.z = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
    }
    return 0;
}

/**
 * @brief 物理对象：设置加速度
 */
static int l_phys_obj_set_acceleration(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    if (lua_gettop(L) >= 4) {
        obj->acceleration.x = luaL_checknumber(L, 2);
        obj->acceleration.y = luaL_checknumber(L, 3);
        obj->acceleration.z = luaL_checknumber(L, 4);
    } else if (lua_gettop(L) >= 2 && lua_istable(L, 2)) {
        lua_getfield(L, 2, "x");
        obj->acceleration.x = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
        lua_getfield(L, 2, "y");
        obj->acceleration.y = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
        lua_getfield(L, 2, "z");
        obj->acceleration.z = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
    }
    return 0;
}

/**
 * @brief 物理对象：设置质量
 */
static int l_phys_obj_set_mass(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    obj->mass = luaL_checknumber(L, 2);
    return 0;
}

/**
 * @brief 物理对象：设置电荷
 */
static int l_phys_obj_set_charge(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    obj->charge = luaL_checknumber(L, 2);
    return 0;
}

/**
 * @brief 物理对象：获取动能
 */
static int l_phys_obj_get_kinetic_energy(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    double v = vector_magnitude(obj->velocity);
    double result = 0.5 * obj->mass * v * v;
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief 物理对象：获取动量（标量）
 */
static int l_phys_obj_get_momentum(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    double v = vector_magnitude(obj->velocity);
    double result = obj->mass * v;
    lua_pushnumber(L, result);
    return 1;
}

/**
 * @brief 物理对象：获取动能（向量版）
 */
static int l_phys_obj_get_kinetic_energy_vector(lua_State* L) {
    (void)L;
    return 0;
}

/**
 * @brief 物理对象：获取动量（向量）
 */
static int l_phys_obj_get_momentum_vector(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    PhysicsVector* result = lua_new_vector(L);
    result->x = obj->mass * obj->velocity.x;
    result->y = obj->mass * obj->velocity.y;
    result->z = obj->mass * obj->velocity.z;
    return 1;
}

/**
 * @brief 物理对象：更新状态
 */
static int l_phys_obj_update(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    double dt = luaL_checknumber(L, 2);
    
    obj->velocity.x += obj->acceleration.x * dt;
    obj->velocity.y += obj->acceleration.y * dt;
    obj->velocity.z += obj->acceleration.z * dt;
    
    obj->position.x += obj->velocity.x * dt;
    obj->position.y += obj->velocity.y * dt;
    obj->position.z += obj->velocity.z * dt;
    
    return 0;
}

/**
 * @brief 物理对象：应用力
 */
static int l_phys_obj_apply_force(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    double fx = 0, fy = 0, fz = 0;
    
    if (lua_gettop(L) >= 4) {
        fx = luaL_checknumber(L, 2);
        fy = luaL_checknumber(L, 3);
        fz = luaL_checknumber(L, 4);
    } else if (lua_gettop(L) >= 2 && lua_istable(L, 2)) {
        lua_getfield(L, 2, "x");
        fx = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
        lua_getfield(L, 2, "y");
        fy = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
        lua_getfield(L, 2, "z");
        fz = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);
    }
    
    if (obj->mass > 0) {
        obj->acceleration.x += fx / obj->mass;
        obj->acceleration.y += fy / obj->mass;
        obj->acceleration.z += fz / obj->mass;
    }
    
    return 0;
}

/**
 * @brief 物理对象：获取位置
 */
static int l_phys_obj_get_position(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    PhysicsVector* result = lua_new_vector(L);
    result->x = obj->position.x;
    result->y = obj->position.y;
    result->z = obj->position.z;
    return 1;
}

/**
 * @brief 物理对象：获取速度
 */
static int l_phys_obj_get_velocity(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    PhysicsVector* result = lua_new_vector(L);
    result->x = obj->velocity.x;
    result->y = obj->velocity.y;
    result->z = obj->velocity.z;
    return 1;
}

/**
 * @brief 物理对象：获取加速度
 */
static int l_phys_obj_get_acceleration(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    PhysicsVector* result = lua_new_vector(L);
    result->x = obj->acceleration.x;
    result->y = obj->acceleration.y;
    result->z = obj->acceleration.z;
    return 1;
}

/**
 * @brief 物理对象：获取质量
 */
static int l_phys_obj_get_mass(lua_State* L) {
    PhysicsObject* obj = luaL_check_phys_obj(L, 1);
    lua_pushnumber(L, obj->mass);
    return 1;
}

/**
 * @brief 打印帮助信息
 */
static int l_physics_help(lua_State* L) {
    const char* func_name = NULL;
    int top = lua_gettop(L);
    
    if (top >= 1 && !lua_isnil(L, 1)) {
        func_name = luaL_checkstring(L, 1);
    }
    
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    
    if (func_name == NULL || strcmp(func_name, "all") == 0) {
        luaL_addstring(&b, "\n========================================\n");
        luaL_addstring(&b, "       物理模块 (physics) 帮助文档 v1.1\n");
        luaL_addstring(&b, "========================================\n\n");
        
        luaL_addstring(&b, "一、模块概述\n");
        luaL_addstring(&b, "----------------------------------------\n");
        luaL_addstring(&b, "physics模块提供了物理世界基本属性的计算功能，\n");
        luaL_addstring(&b, "包括力学、热学、波动、电磁学、光学、相对论等。\n\n");
        
        luaL_addstring(&b, "二、功能分类\n");
        luaL_addstring(&b, "----------------------------------------\n");
        luaL_addstring(&b, "1. 向量运算: vector, add, sub, mul, dot, cross, mag, normalize, lerp, reflect\n");
        luaL_addstring(&b, "2. 力学计算: kinetic_energy, potential_energy, momentum, force, impulse, collision\n");
        luaL_addstring(&b, "3. 转动力学: rotational_kinetic, angular_momentum, torque, centripetal_force\n");
        luaL_addstring(&b, "4. 功与能: work, power, gravity, hooke\n");
        luaL_addstring(&b, "5. 万有引力: gravitational_force, coulomb_force\n");
        luaL_addstring(&b, "6. 振动: shm_displacement, shm_velocity, shm_period, shm_frequency\n");
        luaL_addstring(&b, "7. 波动: wave_frequency, wave_wavelength, wave_energy, wave_intensity\n");
        luaL_addstring(&b, "8. 热学: ideal_gas_law, entropy_change, specific_heat, heat_conduction\n");
        luaL_addstring(&b, "9. 电学: electric_field, electric_potential, capacitance, ohms_law\n");
        luaL_addstring(&b, "10. 光学: refractive_index, snells_law, critical_angle, lens_power, doppler_shift\n");
        luaL_addstring(&b, "11. 相对论: relativistic_mass, relativistic_energy, relativistic_momentum\n");
        luaL_addstring(&b, "12. 量子: photon_energy, de_broglie, compton_shift\n");
        luaL_addstring(&b, "13. 物理对象: new_object, set_position, set_velocity, apply_force\n");
        luaL_addstring(&b, "14. 常量: constants\n");
        luaL_addstring(&b, "15. 单位转换: to_degrees, to_radians\n\n");
        
        luaL_addstring(&b, "三、新增功能\n");
        luaL_addstring(&b, "----------------------------------------\n");
        luaL_addstring(&b, "1. 扩展物理常量（玻尔兹曼常数、阿伏伽德罗常数等）\n");
        luaL_addstring(&b, "2. 向量插值和反射运算\n");
        luaL_addstring(&b, "3. 冲量、碰撞、转动力学计算\n");
        luaL_addstring(&b, "4. 简谐运动周期和频率\n");
        luaL_addstring(&b, "5. 波的能量和强度\n");
        luaL_addstring(&b, "6. 热力学计算（熵变、比热容、热传导）\n");
        luaL_addstring(&b, "7. 电场、电势、电容、欧姆定律\n");
        luaL_addstring(&b, "8. 光学计算（折射、透镜、多普勒、康普顿散射）\n");
        luaL_addstring(&b, "9. 相对论动量、时间膨胀、长度收缩\n");
        luaL_addstring(&b, "10. 物理对象方法（设置/获取状态、应用力、更新位置）\n\n");
        
        luaL_addstring(&b, "四、使用示例\n");
        luaL_addstring(&b, "----------------------------------------\n");
        luaL_addstring(&b, "-- 向量运算\n");
        luaL_addstring(&b, "local v1 = physics.vector(1, 2, 3)\n");
        luaL_addstring(&b, "local v2 = physics.vector(4, 5, 6)\n");
        luaL_addstring(&b, "local v3 = physics.add(v1, v2)\n");
        luaL_addstring(&b, "local dot = physics.dot(v1, v2)\n\n");
        
        luaL_addstring(&b, "-- 力学计算\n");
        luaL_addstring(&b, "local ke = physics.kinetic_energy(2, 10)  -- 100 J\n");
        luaL_addstring(&b, "local pe = physics.potential_energy(5, 10)  -- 490.33 J\n");
        luaL_addstring(&b, "local F = physics.force(2, 9.8)  -- 19.6 N\n\n");
        
        luaL_addstring(&b, "-- 相对论效应\n");
        luaL_addstring(&b, "local c = physics.constants().speed_of_light\n");
        luaL_addstring(&b, "local m = physics.relativistic_mass(1, 0.99 * c)\n");
        luaL_addstring(&b, "local t = physics.time_dilation(1, 0.99 * c, c)\n\n");
        
        luaL_addstring(&b, "-- 物理对象\n");
        luaL_addstring(&b, "local obj = physics.new_object(5, 0)  -- 质量5kg\n");
        luaL_addstring(&b, "obj:set_position(0, 0, 0)\n");
        luaL_addstring(&b, "obj:set_velocity(10, 0, 0)\n");
        luaL_addstring(&b, "obj:apply_force(0, -49, 0)  -- 重力\n");
        luaL_addstring(&b, "obj:update(0.1)  -- 更新状态\n\n");
        
        luaL_addstring(&b, "========================================\n");
        luaL_addstring(&b, "    输入 physics.help('函数名') 查看详情\n");
        luaL_addstring(&b, "========================================\n");
    } else {
        luaL_addstring(&b, "\n[");
        luaL_addstring(&b, func_name);
        luaL_addstring(&b, "]\n");
        luaL_addstring(&b, "----------------------------------------\n");
        
        if (strcmp(func_name, "vector") == 0 || strcmp(func_name, "new_vector") == 0) {
            luaL_addstring(&b, "函数签名: physics.vector(x[, y[, z]])\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  创建三维向量对象。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local v = physics.vector(1, 2, 3)\n");
            luaL_addstring(&b, "  local origin = physics.vector()\n\n");
        } else if (strcmp(func_name, "constants") == 0) {
            luaL_addstring(&b, "函数签名: physics.constants()\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  获取所有物理常量的表。\n\n");
            luaL_addstring(&b, "新增常量:\n");
            luaL_addstring(&b, "  - boltzmann_constant: 玻尔兹曼常数\n");
            luaL_addstring(&b, "  - avogadro_number: 阿伏伽德罗常数\n");
            luaL_addstring(&b, "  - elementary_charge: 元电荷\n");
            luaL_addstring(&b, "  - neutron_mass: 中子质量\n");
            luaL_addstring(&b, "  - bohr_magneton: 玻尔磁子\n");
            luaL_addstring(&b, "  - faraday_constant: 法拉第常数\n\n");
        } else if (strcmp(func_name, "impulse") == 0) {
            luaL_addstring(&b, "函数签名: physics.impulse(force, time)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  计算冲量。\n\n");
            luaL_addstring(&b, "公式: I = F * t\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local I = physics.impulse(100, 0.1)  -- 10 N·s\n\n");
        } else if (strcmp(func_name, "collision") == 0) {
            luaL_addstring(&b, "函数签名: physics.collision(m1, v1, m2, v2[, elastic])\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  计算碰撞后的速度（完全弹性或非弹性）。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local v = physics.collision(1, 10, 2, -5, true)  -- 弹性碰撞\n\n");
        } else if (strcmp(func_name, "rotational_kinetic") == 0) {
            luaL_addstring(&b, "函数签名: physics.rotational_kinetic(I, omega)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  计算转动动能。\n\n");
            luaL_addstring(&b, "公式: Ek = 1/2 * I * ω²\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b,  "  local Ek = physics.rotational_kinetic(0.5, 10)\n\n");
        } else if (strcmp(func_name, "relativistic_momentum") == 0) {
            luaL_addstring(&b, "函数签名: physics.relativistic_momentum(m0, v[, c])\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  计算相对论动量。\n\n");
            luaL_addstring(&b, "公式: p = γ * m₀ * v\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local p = physics.relativistic_momentum(1, 0.99 * 3e8)\n\n");
        } else if (strcmp(func_name, "electric_field") == 0) {
            luaL_addstring(&b, "函数签名: physics.electric_field([k, ]Q, r)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  计算点电荷的电场强度。\n\n");
            luaL_addstring(&b, "公式: E = k * |Q| / r²\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local E = physics.electric_field(1e-6, 0.1)\n\n");
        } else if (strcmp(func_name, "snells_law") == 0) {
            luaL_addstring(&b, "函数签名: physics.snells_law(n1, n2, theta1)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  计算斯涅尔定律（折射角）。\n\n");
            luaL_addstring(&b, "公式: n₁·sin(θ₁) = n₂·sin(θ₂)\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local theta2 = physics.snells_law(1.0, 1.5, 0.5)\n\n");
        } else if (strcmp(func_name, "doppler_shift") == 0) {
            luaL_addstring(&b, "函数签名: physics.doppler_shift(v, c, f0[, toward])\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  计算多普勒频移后的频率。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local f = physics.doppler_shift(30, 340, 440, true)  -- 声源靠近\n\n");
        } else {
            luaL_addstring(&b, "\n[错误] 未找到函数 '");
            luaL_addstring(&b, func_name);
            luaL_addstring(&b, "'\n\n");
            luaL_addstring(&b, "可用函数: vector, scalar, constants\n");
            luaL_addstring(&b, "          kinetic_energy, potential_energy, momentum, force, work, power\n");
            luaL_addstring(&b, "          impulse, collision, rotational_kinetic, angular_momentum, torque\n");
            luaL_addstring(&b, "          gravity, hooke, gravitational_force, coulomb_force\n");
            luaL_addstring(&b, "          shm_displacement, shm_velocity, shm_acceleration, shm_period\n");
            luaL_addstring(&b, "          wave_frequency, wave_wavelength, wave_energy, wave_intensity\n");
            luaL_addstring(&b, "          ideal_gas_law, entropy_change, specific_heat, heat_conduction\n");
            luaL_addstring(&b, "          electric_field, electric_potential, capacitance, ohms_law\n");
            luaL_addstring(&b, "          refractive_index, snells_law, critical_angle, lens_power\n");
            luaL_addstring(&b, "          doppler_shift, compton_shift\n");
            luaL_addstring(&b, "          relativistic_mass, relativistic_energy, relativistic_momentum\n");
            luaL_addstring(&b, "          time_dilation, length_contraction\n");
            luaL_addstring(&b, "          photon_energy, de_broglie\n");
            luaL_addstring(&b, "          to_degrees, to_radians, help\n\n");
        }
    }
    
    luaL_pushresult(&b);
    return 1;
}

/**
 * @brief 设置模块信息
 */
static void set_info(lua_State* L) {
    lua_pushliteral(L, "_COPYRIGHT");
    lua_pushliteral(L, "Copyright (C) 2026 DifierLine");
    lua_settable(L, -3);
    
    lua_pushliteral(L, "_DESCRIPTION");
    lua_pushliteral(L, "Physics module for Lua v1.1 - 物理世界基本属性与计算");
    lua_settable(L, -3);
    
    lua_pushliteral(L, "_VERSION");
    lua_pushliteral(L, PHYSICS_VERSION);
    lua_settable(L, -3);
}

/**
 * @brief 模块注册表
 */
static const struct luaL_Reg physicslib[] = {
    {"new_vector", l_physics_new_vector},
    {"new_scalar", l_physics_new_scalar},
    {"new_object", l_physics_new_object},
    {"vector", l_physics_vector},
    {"scalar", l_physics_scalar},
    {"constants", l_physics_constants},
    {"kinetic_energy", l_physics_kinetic_energy},
    {"potential_energy", l_physics_potential_energy},
    {"momentum", l_physics_momentum},
    {"force", l_physics_force},
    {"work", l_physics_work},
    {"power", l_physics_power},
    {"pressure", l_physics_pressure},
    {"gravity", l_physics_gravity},
    {"hooke", l_physics_hooke},
    {"gravitational_force", l_physics_gravitational_force},
    {"coulomb_force", l_physics_coulomb_force},
    {"impulse", l_physics_impulse},
    {"collision", l_physics_collision},
    {"rotational_kinetic", l_physics_rotational_kinetic},
    {"angular_momentum", l_physics_angular_momentum},
    {"torque", l_physics_torque},
    {"centripetal_force", l_physics_centripetal_force},
    {"centripetal_acceleration", l_physics_centripetal_acceleration},
    {"shm_displacement", l_physics_shm_displacement},
    {"shm_velocity", l_physics_shm_velocity},
    {"shm_acceleration", l_physics_shm_acceleration},
    {"shm_period", l_physics_shm_period},
    {"shm_frequency", l_physics_shm_frequency},
    {"wave_frequency", l_physics_wave_frequency},
    {"wave_wavelength", l_physics_wave_wavelength},
    {"wave_period", l_physics_wave_period},
    {"wave_energy", l_physics_wave_energy},
    {"wave_intensity", l_physics_wave_intensity},
    {"ideal_gas_law", l_physics_ideal_gas_law},
    {"entropy_change", l_physics_entropy_change},
    {"specific_heat", l_physics_specific_heat},
    {"heat_conduction", l_physics_heat_conduction},
    {"relativistic_mass", l_physics_relativistic_mass},
    {"relativistic_energy", l_physics_relativistic_energy},
    {"relativistic_momentum", l_physics_relativistic_momentum},
    {"time_dilation", l_physics_time_dilation},
    {"length_contraction", l_physics_length_contraction},
    {"electric_field", l_physics_electric_field},
    {"electric_potential", l_physics_electric_potential},
    {"electric_potential_energy", l_physics_electric_potential_energy},
    {"capacitance", l_physics_capacitance},
    {"ohms_law", l_physics_ohms_law},
    {"electric_power", l_physics_electric_power},
    {"resistivity", l_physics_resistivity},
    {"refractive_index", l_physics_refractive_index},
    {"snells_law", l_physics_snells_law},
    {"critical_angle", l_physics_critical_angle},
    {"lens_power", l_physics_lens_power},
    {"doppler_shift", l_physics_doppler_shift},
    {"compton_shift", l_physics_compton_shift},
    {"photon_energy", l_physics_photon_energy},
    {"de_broglie", l_physics_de_broglie},
    {"to_degrees", l_physics_to_degrees},
    {"to_radians", l_physics_to_radians},
    {"add", l_physics_vector_add},
    {"sub", l_physics_vector_sub},
    {"mul", l_physics_vector_scale},
    {"dot", l_physics_vector_dot},
    {"cross", l_physics_vector_cross},
    {"mag", l_physics_vector_magnitude},
    {"normalize", l_physics_vector_normalize},
    {"distance", l_physics_vector_distance},
    {"angle", l_physics_vector_angle},
    {"lerp", l_physics_vector_lerp},
    {"reflect", l_physics_vector_reflect},
    {"help", l_physics_help},
    {NULL, NULL}
};

/**
 * @brief 向量方法注册表
 */
static const struct luaL_Reg vector_methods[] = {
    {"add", l_physics_vector_add},
    {"sub", l_physics_vector_sub},
    {"mul", l_physics_vector_scale},
    {"dot", l_physics_vector_dot},
    {"cross", l_physics_vector_cross},
    {"mag", l_physics_vector_magnitude},
    {"normalize", l_physics_vector_normalize},
    {"distance", l_physics_vector_distance},
    {"angle", l_physics_vector_angle},
    {"lerp", l_physics_vector_lerp},
    {"reflect", l_physics_vector_reflect},
    {"getX", l_vector_get_x},
    {"getY", l_vector_get_y},
    {"getZ", l_vector_get_z},
    {"setX", l_vector_set_x},
    {"setY", l_vector_set_y},
    {"setZ", l_vector_set_z},
    {NULL, NULL}
};

/**
 * @brief 向量元方法表
 */
static const struct luaL_Reg vector_meta[] = {
    {"__gc", l_vector_gc},
    {"__tostring", l_vector_tostring},
    {"__add", l_physics_vector_add},
    {"__sub", l_physics_vector_sub},
    {"__mul", l_physics_vector_scale},
    {"__unm", l_physics_vector_scale},
    {NULL, NULL}
};

/**
 * @brief 标量方法注册表
 */
static const struct luaL_Reg scalar_methods[] = {
    {"value", l_scalar_get_value},
    {"unit", l_scalar_get_unit},
    {NULL, NULL}
};

/**
 * @brief 标量元方法表
 */
static const struct luaL_Reg scalar_meta[] = {
    {"__gc", l_scalar_gc},
    {"__tostring", l_scalar_tostring},
    {NULL, NULL}
};

/**
 * @brief 物理对象方法注册表
 */
static const struct luaL_Reg phys_obj_methods[] = {
    {"setPosition", l_phys_obj_set_position},
    {"setVelocity", l_phys_obj_set_velocity},
    {"setAcceleration", l_phys_obj_set_acceleration},
    {"setMass", l_phys_obj_set_mass},
    {"setCharge", l_phys_obj_set_charge},
    {"getKineticEnergy", l_phys_obj_get_kinetic_energy},
    {"getMomentum", l_phys_obj_get_momentum},
    {"getMomentumVector", l_phys_obj_get_momentum_vector},
    {"update", l_phys_obj_update},
    {"applyForce", l_phys_obj_apply_force},
    {"getPosition", l_phys_obj_get_position},
    {"getVelocity", l_phys_obj_get_velocity},
    {"getAcceleration", l_phys_obj_get_acceleration},
    {"getMass", l_phys_obj_get_mass},
    {NULL, NULL}
};

/**
 * @brief 物理对象元方法表
 */
static const struct luaL_Reg phys_obj_meta[] = {
    {"__gc", l_phys_obj_gc},
    {"__tostring", l_phys_obj_tostring},
    {NULL, NULL}
};

/**
 * @brief 模块初始化函数
 */
int luaopen_physics(lua_State* L) {
    luaL_newmetatable(L, VECTOR_METATABLE);
    luaL_setfuncs(L, vector_meta, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, vector_methods, 0);
    
    luaL_newmetatable(L, SCALAR_METATABLE);
    luaL_setfuncs(L, scalar_meta, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, scalar_methods, 0);
    
    luaL_newmetatable(L, PHYS_OBJ_METATABLE);
    luaL_setfuncs(L, phys_obj_meta, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, phys_obj_methods, 0);
    
    luaL_newlib(L, physicslib);
    set_info(L);
    
    return 1;
}
