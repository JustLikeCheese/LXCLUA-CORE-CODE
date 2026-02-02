/**
 * @file tensor.h
 * @brief Lua张量模块头文件 - 版本2.0
 * @author 张量模块
 * @version 2.0
 * @date 2024
 */

#ifndef LUA_TENSOR_H
#define LUA_TENSOR_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define TENSOR_MAX_DIMS 16

/**
 * @brief 张量数据类型枚举
 */
typedef enum {
    TENSOR_INT8 = 0,
    TENSOR_INT16,
    TENSOR_INT32,
    TENSOR_INT64,
    TENSOR_UINT8,
    TENSOR_UINT16,
    TENSOR_UINT32,
    TENSOR_UINT64,
    TENSOR_FLOAT32,
    TENSOR_FLOAT64,
    TENSOR_BOOL
} TensorDataType;

/**
 * @brief 张量结构体
 */
typedef struct {
    int ndims;                          // 维度数量
    int64_t shape[TENSOR_MAX_DIMS];     // 各维度大小
    int64_t stride[TENSOR_MAX_DIMS];    // 各维度步长
    TensorDataType dtype;               // 数据类型
    void* data;                         // 数据指针
    int64_t size;                       // 元素总数
    int64_t itemsize;                   // 每个元素大小（字节）
    int8_t owner;                       // 是否拥有数据所有权
} Tensor;

/**
 * @brief 创建新的张量对象
 * @param L Lua状态机
 * @param shape_len 形状长度
 * @param dtype 数据类型字符串
 * @return 张量对象（作为userdata）
 */
static Tensor* lua_new_tensor_obj(lua_State* L, lua_Integer shape_len, const char* dtype);

/**
 * @brief 获取张量元素大小
 * @param dtype 数据类型
 * @return 元素大小（字节）
 */
int tensor_get_itemsize(TensorDataType dtype);

/**
 * @brief 初始化张量的步长信息
 * @param tensor 张量对象
 */
void tensor_init_strides(Tensor* tensor);

/**
 * @brief 检查参数是否为张量
 * @param L Lua状态机
 * @param index 参数索引
 * @return 张量指针，失败则触发Lua错误
 */
Tensor* luaL_check_tensor(lua_State* L, int index);

/**
 * @brief 释放张量内存
 * @param tensor 张量对象
 */
void tensor_free(Tensor* tensor);

/**
 * @brief 将多维索引转换为一维索引
 * @param tensor 张量对象
 * @param indices 索引数组
 * @return 一维索引，-1表示越界
 */
int64_t tensor_multidim_to_linear(Tensor* tensor, int64_t* indices);

/**
 * @brief 获取元素指针
 * @param tensor 张量对象
 * @param index 线性索引
 * @return 元素指针
 */
void* tensor_get_element_ptr(Tensor* tensor, int64_t index);

/**
 * @brief 复制张量数据
 * @param src 源张量
 * @return 新的张量副本
 */
static Tensor* tensor_copy_data(Tensor* src);

/**
 * @brief 广播操作检查
 * @param t1 第一个张量
 * @param t2 第二个张量
 * @param output_shape 输出形状
 * @return 是否可广播
 */
static int tensor_can_broadcast(Tensor* t1, Tensor* t2, int64_t* output_shape);

/**
 * @brief 元素级二元运算
 * @param t1 第一个张量
 * @param t2 第二个张量
 * @param op 运算符函数指针
 * @return 结果张量
 */
static Tensor* tensor_elementwise_binary(Tensor* t1, Tensor* t2, 
                                          void (*op)(void*, const void*, const void*));

/**
 * @brief 标量运算函数
 */
static void tensor_op_add(void* out, const void* a, const void* b);
static void tensor_op_sub(void* out, const void* a, const void* b);
static void tensor_op_mul(void* out, const void* a, const void* b);
static void tensor_op_div(void* out, const void* a, const void* b);

/**
 * @brief 归约运算函数
 */
static void tensor_reduce_sum(void* out, const void* data, int64_t count);
static void tensor_reduce_max(void* out, const void* data, int64_t count);
static void tensor_reduce_min(void* out, const void* data, int64_t count);

/**
 * @brief 归约运算
 * @param tensor 输入张量
 * @param axis 要归约的维度，-1表示所有维度
 * @param op 归约函数
 * @return 结果张量
 */
static Tensor* tensor_reduce(Tensor* tensor, int axis, 
                              void (*op)(void*, const void*, int64_t));

/**
 * @brief 创建张量并分配内存
 * @param L Lua状态机
 * @param ndims 维度数量
 * @param shape 形状数组
 * @param dtype 数据类型
 * @return 张量对象
 */
static Tensor* tensor_create(lua_State* L, int ndims, int64_t* shape, TensorDataType dtype);

#endif /* LUA_TENSOR_H */
