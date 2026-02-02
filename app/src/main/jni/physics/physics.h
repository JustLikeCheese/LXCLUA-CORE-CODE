/**
 * @file physics.h
 * @brief Lua物理模块头文件 - 物理世界基本属性与计算
 * @author 物理模块
 * @version 1.1
 * @date 2024
 */

#ifndef LUA_PHYSICS_H
#define LUA_PHYSICS_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define PHYSICS_VERSION "1.1"
#define PHYSICS_PI 3.14159265358979323846
#define PHYSICS_E 2.71828182845904523536

/**
 * @brief 物理量类型枚举
 */
typedef enum {
    PHYSICS_QUANTITY_SCALAR = 0,
    PHYSICS_QUANTITY_VECTOR,
    PHYSICS_QUANTITY_TENSOR
} PhysicsQuantityType;

/**
 * @brief 单位系统
 */
typedef enum {
    UNIT_SI = 0,
    UNIT_CGS,
    UNIT_IMPERIAL
} UnitSystem;

/**
 * @brief 向量结构体（用于表示位置、速度、力等）
 */
typedef struct {
    double x;
    double y;
    double z;
} PhysicsVector;

/**
 * @brief 标量物理量结构体
 */
typedef struct {
    double value;
    const char* unit;
} PhysicsScalar;

/**
 * @brief 物理对象结构体
 */
typedef struct {
    PhysicsVector position;
    PhysicsVector velocity;
    PhysicsVector acceleration;
    double mass;
    double charge;
    double moment_of_inertia;
    PhysicsVector angular_velocity;
    PhysicsVector angular_momentum;
} PhysicsObject;

/**
 * @brief 物理常量结构体
 */
typedef struct {
    double speed_of_light;
    double gravitational_constant;
    double planck_constant;
    double reduced_planck_constant;
    double boltzmann_constant;
    double avogadro_number;
    double elementary_charge;
    double electron_mass;
    double proton_mass;
    double neutron_mass;
    double earth_gravity;
    double gas_constant;
    double standard_pressure;
    double standard_temperature;
    double stefan_boltzmann_constant;
    double vacuum_permeability;
    double vacuum_permittivity;
    double bohr_magneton;
    double bohr_radius;
    double rydberg_constant;
    double faraday_constant;
    double wien_displacement;
    double stephen_boltzmann;
} PhysicalConstants;

/**
 * @brief 波属性结构体
 */
typedef struct {
    double wavelength;
    double frequency;
    double amplitude;
    double period;
    double wave_number;
    double angular_frequency;
    double phase;
    double speed;
} WaveProperties;

/**
 * @brief 热力学属性结构体
 */
typedef struct {
    double temperature;
    double pressure;
    double volume;
    double internal_energy;
    double entropy;
    double enthalpy;
    double specific_heat_cv;
    double specific_heat_cp;
} ThermodynamicProperties;

/**
 * @brief 电学属性结构体
 */
typedef struct {
    double voltage;
    double current;
    double resistance;
    double capacitance;
    double inductance;
    double charge;
    double electric_field;
    double potential;
} ElectricalProperties;

PhysicsVector* lua_new_vector(lua_State* L);
PhysicsVector* luaL_check_vector(lua_State* L, int index);
PhysicsScalar* lua_new_scalar(lua_State* L);
PhysicsScalar* luaL_check_scalar(lua_State* L, int index);
PhysicsObject* lua_new_phys_obj(lua_State* L);
PhysicsObject* luaL_check_phys_obj(lua_State* L, int index);

PhysicsVector vector_add(PhysicsVector a, PhysicsVector b);
PhysicsVector vector_sub(PhysicsVector a, PhysicsVector b);
PhysicsVector vector_scale(PhysicsVector v, double scalar);
double vector_dot(PhysicsVector a, PhysicsVector b);
PhysicsVector vector_cross(PhysicsVector a, PhysicsVector b);
double vector_magnitude(PhysicsVector v);
PhysicsVector vector_normalize(PhysicsVector v);
double vector_distance(PhysicsVector a, PhysicsVector b);
double vector_angle(PhysicsVector a, PhysicsVector b);
double vector_scalar_triple(PhysicsVector a, PhysicsVector b, PhysicsVector c);
PhysicsVector vector_lerp(PhysicsVector a, PhysicsVector b, double t);
PhysicsVector vector_reflect(PhysicsVector v, PhysicsVector normal);

double radians_to_degrees(double radians);
double degrees_to_radians(double degrees);

double calculate_kinetic_energy(double mass, double velocity);
double calculate_potential_energy(double mass, double height, double g);
double calculate_momentum(double mass, double velocity);
double calculate_force(double mass, double acceleration);
double calculate_work(double force, double distance, double angle);
double calculate_power(double work, double time);
double calculate_pressure(double force, double area);
double calculate_density(double mass, double volume);
double calculate_gravity(double mass, double g);
double calculate_hooke_force(double k, double x);
double calculate_gravitational_force(double m1, double m2, double r, double G);
double calculate_coulomb_force(double q1, double q2, double r, double k);
double calculate_impulse(double force, double time);
double calculate_collision_velocity(double m1, double v1, double m2, double v2, int elastic);
double calculate_rotational_kinetic(double I, double omega);
double calculate_angular_momentum(double I, double omega);
double calculate_torque(double r, double F, double angle);
double calculate_centripetal_force(double m, double v, double r);
double calculate_centripetal_acceleration(double v, double r);

double shm_displacement(double amplitude, double angular_frequency, double time, double phase);
double shm_velocity(double amplitude, double angular_frequency, double time, double phase);
double shm_acceleration(double amplitude, double angular_frequency, double time, double phase);
double shm_period(double mass, double k);
double shm_frequency(double mass, double k);
double shm_angular_frequency(double mass, double k);

double wave_frequency(double speed, double wavelength);
double wave_wavelength(double speed, double frequency);
double wave_period(double frequency);
double wave_energy(double amplitude, double frequency, double mass_density);
double wave_intensity(double power, double area);

double ideal_gas_pressure(double n, double R, double T, double V);
double ideal_gas_law(double P, double V, double n, double R, double T, int solve_for_P);
double calculate_entropy_change(double Q, double T);
double calculate_specific_heat(double Q, double m, double delta_T);
double calculate_heat_conduction(double k, double A, double dT, double d);
double calculate_heat_capacity(double Q, double delta_T);

double stefan_boltzmann_power(double sigma, double area, double T, double emissivity);
double de_broglie_wavelength(double h, double momentum);
double photon_energy(double h, double frequency);
double photon_momentum(double h, double wavelength);
double relativistic_mass(double rest_mass, double velocity, double c);
double relativistic_energy(double rest_mass, double velocity, double c);
double relativistic_momentum(double rest_mass, double velocity, double c);
double time_dilation(double proper_time, double velocity, double c);
double length_contraction(double proper_length, double velocity, double c);

double calculate_electric_field(double k, double Q, double r);
double calculate_electric_potential(double k, double Q, double r);
double calculate_electric_potential_energy(double k, double Q1, double Q2, double r);
double calculate_capacitance(double Q, double V);
double calculate_ohms_law(double V, double I, double R, int solve_for_V);
double calculate_electric_power(double V, double I, double R);
double calculate_resistivity(double R, double L, double A);
double calculate_conductivity(double rho);

double calculate_refractive_index(double c, double v);
double calculate_snells_law(double n1, double n2, double theta1);
double calculate_critical_angle(double n1, double n2);
double calculate_lens_power(double f);
double calculate_thin_lens(double f, double do_di, int solve_for_di);
double calculate_doppler_shift(double v, double c, double f0, int source_moving_toward);
double calculate_compton_shift(double theta, double h, double m, double c);

PhysicalConstants get_physical_constants(void);
void physics_object_free(PhysicsObject* obj);

int l_physics_vector_x(lua_State* L);
int l_physics_vector_y(lua_State* L);
int l_physics_vector_z(lua_State* L);
int l_physics_vector_set_x(lua_State* L);
int l_physics_vector_set_y(lua_State* L);
int l_physics_vector_set_z(lua_State* L);

int l_physics_scalar_value(lua_State* L);
int l_physics_scalar_unit(lua_State* L);

int l_physics_obj_set_position(lua_State* L);
int l_physics_obj_set_velocity(lua_State* L);
int l_physics_obj_set_acceleration(lua_State* L);
int l_physics_obj_set_mass(lua_State* L);
int l_physics_obj_set_charge(lua_State* L);
int l_physics_obj_get_kinetic_energy(lua_State* L);
int l_physics_obj_get_momentum(lua_State* L);
int l_physics_obj_get_kinetic_energy_vector(lua_State* L);
int l_physics_obj_get_momentum_vector(lua_State* L);
int l_physics_obj_update(lua_State* L);
int l_physics_obj_apply_force(lua_State* L);
int l_physics_obj_get_position(lua_State* L);
int l_physics_obj_get_velocity(lua_State* L);
int l_physics_obj_get_acceleration(lua_State* L);
int l_physics_obj_get_mass(lua_State* L);

#endif /* LUA_PHYSICS_H */
