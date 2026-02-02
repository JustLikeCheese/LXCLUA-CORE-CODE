/**
 * @file network.h
 * @brief 神经网络模块头文件
 * @author 神经网络模块
 * @version 1.0
 * @date 2024
 */

#ifndef LUA_NETWORK_H
#define LUA_NETWORK_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define NETWORK_VERSION "1.0"
#define NETWORK_MAX_LAYERS 20
#define NETWORK_MAX_NEURONS 1000

/**
 * @brief 激活函数类型
 */
typedef enum {
    ACTIVATION_SIGMOID = 0,
    ACTIVATION_RELU,
    ACTIVATION_TANH,
    ACTIVATION_LINEAR,
    ACTIVATION_SOFTMAX
} ActivationType;

/**
 * @brief 激活函数信息
 */
typedef struct {
    ActivationType type;
    const char* name;
    double (*forward)(double x);
    double (*derivative)(double x);
} ActivationInfo;

/**
 * @brief 网络层结构
 */
typedef struct Layer {
    int input_size;           // 输入大小
    int output_size;          // 输出大小（神经元数量）
    double* weights;          // 权重矩阵 (output_size × input_size)
    double* biases;           // 偏置向量 (output_size)
    double* outputs;          // 层输出 (output_size)
    double* inputs;           // 层输入 (input_size)
    double* deltas;           // 误差项 (output_size)
    ActivationType activation; // 激活函数类型
    struct Layer* prev;       // 上一层指针
    struct Layer* next;       // 下一层指针
} Layer;

/**
 * @brief 神经网络结构
 */
typedef struct {
    Layer* input_layer;       // 输入层
    Layer* output_layer;      // 输出层
    Layer* layers;            // 所有层的数组
    int layer_count;          // 层数量（不包括输入层）
    int input_size;           // 输入大小
    int output_size;          // 输出大小
    double learning_rate;     // 学习率
    double momentum;          // 动量
    double weight_decay;      // 权重衰减
    int is_trained;           // 是否已训练
    int epoch;                // 训练轮数
    double last_loss;         // 上次损失
} Network;

/**
 * @brief 训练数据对
 */
typedef struct {
    double* input;            // 输入向量
    double* target;           // 目标输出
    int input_size;           // 输入大小
    int output_size;          // 输出大小
} DataPair;

/**
 * @brief 数据集
 */
typedef struct {
    DataPair* pairs;          // 数据对数组
    int count;                // 数据数量
    int input_size;           // 输入大小
    int output_size;          // 输出大小
} Dataset;

/**
 * @brief 创建新的网络对象
 * @param L Lua状态机
 * @return 网络对象
 */
Network* lua_new_network(lua_State* L);

/**
 * @brief 检查参数是否为网络
 * @param L Lua状态机
 * @param index 参数索引
 * @return 网络指针，失败则触发Lua错误
 */
Network* luaL_check_network(lua_State* L, int index);

/**
 * @brief 创建新层
 * @param input_size 输入大小
 * @param output_size 输出大小
 * @param activation 激活函数类型
 * @return 层指针
 */
Layer* layer_create(int input_size, int output_size, ActivationType activation);

/**
 * @brief 释放层内存
 * @param layer 层指针
 */
void layer_free(Layer* layer);

/**
 * @brief 初始化层权重
 * @param layer 层指针
 * @param method 初始化方法: "random", "xavier", "he"
 */
void layer_init_weights(Layer* layer, const char* method);

/**
 * @brief 前向传播
 * @param layer 层指针
 * @param inputs 输入向量
 */
void layer_forward(Layer* layer, double* inputs);

/**
 * @brief 反向传播
 * @param layer 层指针
 * @param learning_rate 学习率
 */
void layer_backward(Layer* layer, double learning_rate);

/**
 * @brief 创建网络
 * @param L Lua状态机
 * @param architecture 网络架构表
 * @param input_size 输入大小
 * @return 网络指针
 */
Network* network_create(lua_State* L, int* architecture, int arch_size, int input_size);

/**
 * @brief 释放网络内存
 * @param network 网络指针
 */
void network_free(Network* network);

/**
 * @brief 网络前向传播
 * @param network 网络指针
 * @param inputs 输入向量
 * @param outputs 输出向量
 */
void network_forward(Network* network, double* inputs, double* outputs);

/**
 * @brief 计算损失
 * @param outputs 输出向量
 * @param targets 目标向量
 * @param output_size 输出大小
 * @param loss_type 损失类型: "mse", "cross_entropy"
 * @return 损失值
 */
double network_loss(double* outputs, double* targets, int output_size, const char* loss_type);

/**
 * @brief 网络训练（单步）
 * @param network 网络指针
 * @param input 输入向量
 * @param target 目标向量
 * @return 损失值
 */
double network_train_step(Network* network, double* input, double* target);

/**
 * @brief 网络训练（多轮）
 * @param network 网络指针
 * @param dataset 数据集
 * @param epochs 训练轮数
 * @param mini_batch_size 小批量大小
 * @return 最终损失
 */
double network_train(Network* network, Dataset* dataset, int epochs, int mini_batch_size);

/**
 * @brief 网络预测
 * @param network 网络指针
 * @param input 输入向量
 * @return 输出向量（Lua表）
 */
int network_predict(lua_State* L, Network* network, double* input);

/**
 * @brief 保存网络到文件
 * @param network 网络指针
 * @param filename 文件路径
 * @return 是否成功
 */
int network_save(Network* network, const char* filename);

/**
 * @brief 从文件加载网络
 * @param L Lua状态机
 * @param filename 文件路径
 * @return 网络对象
 */
Network* network_load(lua_State* L, const char* filename);

/**
 * @brief 获取网络权重为Lua表
 * @param L Lua状态机
 * @param network 网络指针
 * @return 权重表
 */
int network_get_weights(lua_State* L, Network* network);

/**
 * @brief 设置网络权重
 * @param network 网络指针
 * @param weights 权重表
 * @return 是否成功
 */
int network_set_weights(Network* network, lua_State* L, int idx);

/**
 * @brief 创建数据集
 * @param L Lua状态机
 * @param data_table Lua数据表
 * @return 数据集指针
 */
Dataset* dataset_create(lua_State* L, int input_size, int output_size);

/**
 * @brief 保存数据集到文件
 * @param dataset 数据集指针
 * @param filename 文件路径
 * @return 是否成功
 */
int dataset_save(Dataset* dataset, const char* filename);

/**
 * @brief 从文件加载数据集
 * @param L Lua状态机
 * @param filename 文件路径
 * @return 数据集指针
 */
Dataset* dataset_load(lua_State* L, const char* filename);

/**
 * @brief 释放数据集
 * @param dataset 数据集指针
 */
void dataset_free(Dataset* dataset);

/**
 * @brief 获取激活函数
 * @param name 激活函数名称
 * @return 激活函数信息
 */
ActivationInfo get_activation(const char* name);

/**
 * @brief 激活函数实现
 */
double sigmoid(double x);
double sigmoid_derivative(double x);
double relu(double x);
double relu_derivative(double x);
double tanh_func(double x);
double tanh_derivative(double x);
double linear(double x);
double linear_derivative(double x);
double softmax_func(double x);
double softmax_derivative(double x);

/**
 * @brief 归一化数据
 * @param data 数据数组
 * @param size 数据大小
 * @param min_val 最小值
 * @param max_val 最大值
 */
void normalize_data(double* data, int size, double min_val, double max_val);

/**
 * @brief 反归一化数据
 * @param data 数据数组
 * @param size 数据大小
 * @param min_val 最小值
 * @param max_val 最大值
 */
void denormalize_data(double* data, int size, double min_val, double max_val);

#endif /* LUA_NETWORK_H */
