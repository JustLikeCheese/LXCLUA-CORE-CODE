/**
 * @file tensor.c
 * @brief Lua张量模块实现 - 基于Lua表的底层数据结构
 * @author 张量模块
 * @version 2.0
 * @date 2024
 */

#include "tensor.h"
#include <string.h>
#include <math.h>

#define TENSOR_METATABLE "Tensor"

/**
 * @brief 创建一个新的张量对象（Lua表结构）
 * @param L Lua状态机
 * @param shape 形状表
 * @param dtype 数据类型
 * @return 张量对象（Lua表）
 */
static Tensor* lua_new_tensor_obj(lua_State* L, lua_Integer shape_len, const char* dtype) {
    Tensor* tensor = (Tensor*)lua_newuserdata(L, sizeof(Tensor));
    luaL_getmetatable(L, TENSOR_METATABLE);
    lua_setmetatable(L, -2);
    
    tensor->ndims = (int)shape_len;
    tensor->dtype = TENSOR_FLOAT64;
    tensor->itemsize = 8;
    tensor->size = 1;
    tensor->data = NULL;
    tensor->owner = 1;
    
    if (strcmp(dtype, "float32") == 0) {
        tensor->dtype = TENSOR_FLOAT32;
        tensor->itemsize = 4;
    } else if (strcmp(dtype, "int32") == 0) {
        tensor->dtype = TENSOR_INT32;
        tensor->itemsize = 4;
    } else if (strcmp(dtype, "int64") == 0) {
        tensor->dtype = TENSOR_INT64;
        tensor->itemsize = 8;
    }
    
    for (int i = 0; i < TENSOR_MAX_DIMS; i++) {
        tensor->shape[i] = 1;
        tensor->stride[i] = 0;
    }
    
    return tensor;
}

/**
 * @brief 获取张量元素大小
 * @param dtype 数据类型
 * @return 元素大小（字节）
 */
int tensor_get_itemsize(TensorDataType dtype) {
    switch (dtype) {
        case TENSOR_INT8:
        case TENSOR_UINT8:
        case TENSOR_BOOL:
            return 1;
        case TENSOR_INT16:
        case TENSOR_UINT16:
            return 2;
        case TENSOR_INT32:
        case TENSOR_UINT32:
        case TENSOR_FLOAT32:
            return 4;
        case TENSOR_INT64:
        case TENSOR_UINT64:
        case TENSOR_FLOAT64:
            return 8;
        default:
            return 8;
    }
}

/**
 * @brief 计算张量总大小
 * @param L Lua状态机
 * @param shape_idx 形状表在栈中的索引
 * @return 元素总数
 */
static int64_t tensor_calc_size(lua_State* L, int shape_idx) {
    int64_t size = 1;
    int len = luaL_len(L, shape_idx);
    for (int i = 1; i <= len; i++) {
        lua_geti(L, shape_idx, i);
        int64_t dim = (int64_t)lua_tointeger(L, -1);
        size *= dim;
        lua_pop(L, 1);
    }
    return size;
}

/**
 * @brief 初始化张量的步长信息
 * @param tensor 张量对象
 */
void tensor_init_strides(Tensor* tensor) {
    for (int i = 0; i < TENSOR_MAX_DIMS; i++) {
        tensor->stride[i] = 0;
    }
    
    if (tensor->ndims <= 0) return;
    
    int64_t stride = 1;
    for (int i = tensor->ndims - 1; i >= 0; i--) {
        tensor->stride[i] = stride;
        stride *= tensor->shape[i];
    }
}

/**
 * @brief 检查参数是否为张量
 * @param L Lua状态机
 * @param index 参数索引
 * @return 张量指针，失败则触发Lua错误
 */
Tensor* luaL_check_tensor(lua_State* L, int index) {
    Tensor* tensor = (Tensor*)luaL_checkudata(L, index, TENSOR_METATABLE);
    if (tensor == NULL) {
        luaL_argerror(L, index, "expected Tensor");
    }
    return tensor;
}

/**
 * @brief 释放张量内存
 * @param tensor 张量对象
 */
void tensor_free(Tensor* tensor) {
    if (tensor->owner && tensor->data != NULL) {
        free(tensor->data);
        tensor->data = NULL;
    }
}

/**
 * @brief 将多维索引转换为一维索引
 * @param tensor 张量对象
 * @param indices 索引数组
 * @return 一维索引，-1表示越界
 */
int64_t tensor_multidim_to_linear(Tensor* tensor, int64_t* indices) {
    int64_t linear_idx = 0;
    for (int i = 0; i < tensor->ndims; i++) {
        if (indices[i] < 0) {
            indices[i] += tensor->shape[i];
        }
        if (indices[i] < 0 || indices[i] >= tensor->shape[i]) {
            return -1;
        }
        linear_idx += indices[i] * tensor->stride[i];
    }
    return linear_idx;
}

/**
 * @brief 获取元素指针
 * @param tensor 张量对象
 * @param index 线性索引
 * @return 元素指针
 */
void* tensor_get_element_ptr(Tensor* tensor, int64_t index) {
    return (char*)tensor->data + index * tensor->itemsize;
}

/**
 * @brief 设置张量元素值
 * @param tensor 张量对象
 * @param index 线性索引
 * @param value 要设置的值
 */
static void tensor_set_value(Tensor* tensor, int64_t index, double value) {
    void* ptr = tensor_get_element_ptr(tensor, index);
    switch (tensor->dtype) {
        case TENSOR_FLOAT64:
            *(double*)ptr = value;
            break;
        case TENSOR_FLOAT32:
            *(float*)ptr = (float)value;
            break;
        case TENSOR_INT64:
            *(int64_t*)ptr = (int64_t)value;
            break;
        case TENSOR_INT32:
            *(int32_t*)ptr = (int32_t)value;
            break;
        default:
            break;
    }
}

/**
 * @brief 获取张量元素值
 * @param tensor 张量对象
 * @param index 线性索引
 * @return 元素值
 */
static double tensor_get_value(Tensor* tensor, int64_t index) {
    void* ptr = tensor_get_element_ptr(tensor, index);
    switch (tensor->dtype) {
        case TENSOR_FLOAT64:
            return *(double*)ptr;
        case TENSOR_FLOAT32:
            return (double)*(float*)ptr;
        case TENSOR_INT64:
            return (double)*(int64_t*)ptr;
        case TENSOR_INT32:
            return (double)*(int32_t*)ptr;
        default:
            return 0.0;
    }
}

/**
 * @brief 创建张量并分配内存
 * @param L Lua状态机
 * @param shape 形状数组
 * @param ndims 维度数量
 * @param dtype 数据类型
 * @return 张量对象
 */
static Tensor* tensor_create(lua_State* L, int ndims, int64_t* shape, TensorDataType dtype) {
    Tensor* tensor = lua_new_tensor_obj(L, ndims, dtype == TENSOR_FLOAT64 ? "float64" : 
                                                      dtype == TENSOR_FLOAT32 ? "float32" :
                                                      dtype == TENSOR_INT32 ? "int32" : "int64");
    tensor->ndims = ndims;
    tensor->dtype = dtype;
    tensor->itemsize = tensor_get_itemsize(dtype);
    
    for (int i = 0; i < ndims && i < TENSOR_MAX_DIMS; i++) {
        if (shape[i] <= 0) {
            luaL_error(L, "shape dimension %d must be positive", i);
        }
        tensor->shape[i] = shape[i];
    }
    
    tensor->size = 1;
    for (int i = 0; i < ndims; i++) {
        tensor->size *= shape[i];
    }
    
    if (tensor->size > 0) {
        tensor->data = calloc(tensor->size, tensor->itemsize);
        if (tensor->data == NULL) {
            luaL_error(L, "failed to allocate tensor memory");
        }
    } else {
        tensor->data = NULL;
    }
    
    tensor_init_strides(tensor);
    return tensor;
}

/**
 * @brief 复制张量
 * @param src 源张量
 * @return 新的张量副本
 */
static Tensor* tensor_copy_data(Tensor* src) {
    Tensor* dst = (Tensor*)malloc(sizeof(Tensor));
    if (dst == NULL) return NULL;
    
    dst->ndims = src->ndims;
    dst->dtype = src->dtype;
    dst->itemsize = src->itemsize;
    dst->size = src->size;
    dst->owner = 1;
    
    for (int i = 0; i < src->ndims && i < TENSOR_MAX_DIMS; i++) {
        dst->shape[i] = src->shape[i];
        dst->stride[i] = src->stride[i];
    }
    
    if (src->size > 0) {
        dst->data = malloc(src->size * src->itemsize);
        if (dst->data == NULL) {
            free(dst);
            return NULL;
        }
        memcpy(dst->data, src->data, src->size * src->itemsize);
    } else {
        dst->data = NULL;
    }
    
    return dst;
}

/**
 * @brief 广播操作检查并返回输出形状
 * @param t1 第一个张量
 * @param t2 第二个张量
 * @param output_shape 输出形状数组
 * @return 是否可广播
 */
static int tensor_can_broadcast(Tensor* t1, Tensor* t2, int64_t* output_shape) {
    int max_dims = t1->ndims > t2->ndims ? t1->ndims : t2->ndims;
    
    int64_t t1_shape[TENSOR_MAX_DIMS] = {0};
    int64_t t2_shape[TENSOR_MAX_DIMS] = {0};
    
    for (int i = 0; i < TENSOR_MAX_DIMS; i++) {
        if (i < max_dims - t1->ndims) {
            t1_shape[i] = 1;
        } else {
            t1_shape[i] = t1->shape[i - (max_dims - t1->ndims)];
        }
        
        if (i < max_dims - t2->ndims) {
            t2_shape[i] = 1;
        } else {
            t2_shape[i] = t2->shape[i - (max_dims - t2->ndims)];
        }
    }
    
    for (int i = 0; i < max_dims; i++) {
        if (t1_shape[i] == t2_shape[i]) {
            output_shape[i] = t1_shape[i];
        } else if (t1_shape[i] == 1) {
            output_shape[i] = t2_shape[i];
        } else if (t2_shape[i] == 1) {
            output_shape[i] = t1_shape[i];
        } else {
            return 0;
        }
    }
    
    return 1;
}

/**
 * @brief 创建广播后的临时张量视图
 */
static Tensor* tensor_broadcast_view(Tensor* src, int64_t* shape, int ndims) {
    Tensor* dst = (Tensor*)malloc(sizeof(Tensor));
    if (dst == NULL) return NULL;
    
    dst->ndims = ndims;
    dst->dtype = src->dtype;
    dst->itemsize = src->itemsize;
    dst->size = src->size;
    dst->owner = 0;
    
    for (int i = 0; i < ndims && i < TENSOR_MAX_DIMS; i++) {
        dst->shape[i] = shape[i];
    }
    
    dst->data = src->data;
    
    tensor_init_strides(dst);
    
    return dst;
}

/**
 * @brief 标量运算函数
 */
static void tensor_op_add(void* out, const void* a, const void* b) {
    double da = *(double*)a;
    double db = *(double*)b;
    *(double*)out = da + db;
}

static void tensor_op_sub(void* out, const void* a, const void* b) {
    double da = *(double*)a;
    double db = *(double*)b;
    *(double*)out = da - db;
}

static void tensor_op_mul(void* out, const void* a, const void* b) {
    double da = *(double*)a;
    double db = *(double*)b;
    *(double*)out = da * db;
}

static void tensor_op_div(void* out, const void* a, const void* b) {
    double da = *(double*)a;
    double db = *(double*)b;
    *(double*)out = da / db;
}

/**
 * @brief 元素级二元运算
 */
static Tensor* tensor_elementwise_binary(Tensor* t1, Tensor* t2, 
                                          void (*op)(void*, const void*, const void*)) {
    int max_dims = t1->ndims > t2->ndims ? t1->ndims : t2->ndims;
    
    int64_t output_shape[TENSOR_MAX_DIMS];
    if (!tensor_can_broadcast(t1, t2, output_shape)) {
        return NULL;
    }
    
    Tensor* dst = (Tensor*)malloc(sizeof(Tensor));
    if (dst == NULL) return NULL;
    
    dst->ndims = max_dims;
    dst->dtype = TENSOR_FLOAT64;
    dst->itemsize = 8;
    dst->owner = 1;
    
    for (int i = 0; i < max_dims && i < TENSOR_MAX_DIMS; i++) {
        dst->shape[i] = output_shape[i];
    }
    
    dst->size = 1;
    for (int i = 0; i < max_dims; i++) {
        dst->size *= output_shape[i];
    }
    
    dst->data = calloc(dst->size, dst->itemsize);
    
    if (dst->data == NULL) {
        free(dst);
        return NULL;
    }
    
    tensor_init_strides(dst);
    
    Tensor* b1 = tensor_broadcast_view(t1, output_shape, max_dims);
    Tensor* b2 = tensor_broadcast_view(t2, output_shape, max_dims);
    
    for (int64_t i = 0; i < dst->size; i++) {
        void* ptr1 = tensor_get_element_ptr(b1, i);
        void* ptr2 = tensor_get_element_ptr(b2, i);
        void* ptr_out = tensor_get_element_ptr(dst, i);
        op(ptr_out, ptr1, ptr2);
    }
    
    free(b1);
    free(b2);
    
    return dst;
}

/**
 * @brief 归约运算函数
 */
static void tensor_reduce_sum(void* out, const void* data, int64_t count) {
    double sum = 0;
    double* d = (double*)data;
    for (int64_t i = 0; i < count; i++) {
        sum += d[i];
    }
    *(double*)out = sum;
}

static void tensor_reduce_max(void* out, const void* data, int64_t count) {
    double* d = (double*)data;
    double max_val = d[0];
    for (int64_t i = 1; i < count; i++) {
        if (d[i] > max_val) {
            max_val = d[i];
        }
    }
    *(double*)out = max_val;
}

static void tensor_reduce_min(void* out, const void* data, int64_t count) {
    double* d = (double*)data;
    double min_val = d[0];
    for (int64_t i = 1; i < count; i++) {
        if (d[i] < min_val) {
            min_val = d[i];
        }
    }
    *(double*)out = min_val;
}

/**
 * @brief 归约运算
 */
static Tensor* tensor_reduce(Tensor* tensor, int axis, 
                              void (*op)(void*, const void*, int64_t)) {
    Tensor* dst = (Tensor*)malloc(sizeof(Tensor));
    if (dst == NULL) return NULL;
    
    if (axis < 0 || axis >= tensor->ndims) {
        dst->ndims = 0;
        dst->size = 1;
        dst->dtype = TENSOR_FLOAT64;
        dst->itemsize = 8;
        dst->owner = 1;
        dst->data = malloc(tensor->itemsize);
        
        if (dst->data == NULL) {
            free(dst);
            return NULL;
        }
        
        if (tensor->size == 0 || tensor->data == NULL) {
            *(double*)dst->data = 0.0;
        } else {
            void (*scalar_op)(void*, const void*, int64_t);
            scalar_op = op;
            scalar_op(dst->data, tensor->data, tensor->size);
        }
        
        return dst;
    }
    
    dst->ndims = tensor->ndims - 1;
    dst->dtype = TENSOR_FLOAT64;
    dst->itemsize = 8;
    dst->owner = 1;
    
    int j = 0;
    for (int i = 0; i < tensor->ndims; i++) {
        if (i != axis) {
            dst->shape[j] = tensor->shape[i];
            j++;
        }
    }
    
    dst->size = 1;
    for (int i = 0; i < dst->ndims; i++) {
        dst->size *= dst->shape[i];
    }
    
    dst->data = calloc(dst->size, dst->itemsize);
    
    if (dst->data == NULL) {
        free(dst);
        return NULL;
    }
    
    tensor_init_strides(dst);
    
    int64_t outer_dims = 1;
    for (int i = 0; i < axis; i++) {
        outer_dims *= tensor->shape[i];
    }
    
    int64_t inner_dims = 1;
    for (int i = axis + 1; i < tensor->ndims; i++) {
        inner_dims *= tensor->shape[i];
    }
    
    int64_t reduce_dim = tensor->shape[axis];
    
    for (int64_t outer = 0; outer < outer_dims; outer++) {
        for (int64_t inner = 0; inner < inner_dims; inner++) {
            int64_t src_base = outer * tensor->stride[axis] * reduce_dim + 
                              inner * tensor->stride[axis + 1];
            int dst_stride_idx = axis > (tensor->ndims - 2) ? (tensor->ndims - 2) : axis;
            int64_t dst_idx = outer * dst->stride[dst_stride_idx] + inner;
            
            void* dst_ptr = (char*)dst->data + dst_idx * dst->itemsize;
            void* src_ptr = (char*)tensor->data + src_base * tensor->itemsize;
            
            op(dst_ptr, src_ptr, reduce_dim);
        }
    }
    
    return dst;
}

/**
 * @brief tensor.new(shape[, dtype]) - 创建张量
 */
static int l_tensor_new(lua_State* L) {
    int top = lua_gettop(L);
    
    if (top < 1 || !lua_istable(L, 1)) {
        return luaL_error(L, "expected table as shape");
    }
    
    const char* dtype = "float64";
    if (top >= 2) {
        dtype = luaL_checkstring(L, 2);
    }
    
    int ndims = luaL_len(L, 1);
    if (ndims > TENSOR_MAX_DIMS) {
        return luaL_error(L, "too many dimensions (max %d)", TENSOR_MAX_DIMS);
    }
    
    int64_t shape[TENSOR_MAX_DIMS];
    for (int i = 0; i < ndims; i++) {
        lua_geti(L, 1, i + 1);
        shape[i] = (int64_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    
    TensorDataType dtype_enum = TENSOR_FLOAT64;
    if (strcmp(dtype, "float32") == 0) {
        dtype_enum = TENSOR_FLOAT32;
    } else if (strcmp(dtype, "int32") == 0) {
        dtype_enum = TENSOR_INT32;
    } else if (strcmp(dtype, "int64") == 0) {
        dtype_enum = TENSOR_INT64;
    }
    
    tensor_create(L, ndims, shape, dtype_enum);
    return 1;
}

/**
 * @brief tensor.zeros(shape) - 创建零张量
 */
static int l_tensor_zeros(lua_State* L) {
    int top = lua_gettop(L);
    
    if (top < 1 || !lua_istable(L, 1)) {
        return luaL_error(L, "expected table as shape");
    }
    
    int ndims = luaL_len(L, 1);
    int64_t shape[TENSOR_MAX_DIMS];
    for (int i = 0; i < ndims; i++) {
        lua_geti(L, 1, i + 1);
        shape[i] = (int64_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    
    Tensor* tensor = tensor_create(L, ndims, shape, TENSOR_FLOAT64);
    
    return 1;
}

/**
 * @brief tensor.eye(n[, m[, k]]) - 创建单位矩阵
 */
static int l_tensor_eye(lua_State* L) {
    int64_t n = (int64_t)luaL_checkinteger(L, 1);
    int64_t m = n;
    
    if (lua_gettop(L) >= 2) {
        m = (int64_t)luaL_checkinteger(L, 2);
    }
    
    int64_t shape[2] = {n, m};
    Tensor* tensor = tensor_create(L, 2, shape, TENSOR_FLOAT64);
    memset(tensor->data, 0, tensor->size * tensor->itemsize);
    
    int64_t k = 0;
    if (lua_gettop(L) >= 3) {
        k = (int64_t)luaL_checkinteger(L, 3);
    }
    
    for (int64_t i = 0; i < n; i++) {
        int64_t j = i + k;
        if (j >= 0 && j < m) {
            double* ptr = (double*)tensor_get_element_ptr(tensor, i * m + j);
            *ptr = 1.0;
        }
    }
    
    return 1;
}

/**
 * @brief tensor:shape() - 获取张量形状
 */
static int l_tensor_shape(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    lua_createtable(L, tensor->ndims, 0);
    for (int i = 0; i < tensor->ndims; i++) {
        lua_pushinteger(L, tensor->shape[i]);
        lua_seti(L, -2, i + 1);
    }
    
    return 1;
}

/**
 * @brief tensor:ndims() - 获取维度数
 */
static int l_tensor_ndims(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    lua_pushinteger(L, tensor->ndims);
    return 1;
}

/**
 * @brief tensor:size() - 获取元素总数
 */
static int l_tensor_size(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    lua_pushinteger(L, tensor->size);
    return 1;
}

/**
 * @brief tensor:dtype() - 获取数据类型
 */
static int l_tensor_dtype(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    const char* dtype_str;
    switch (tensor->dtype) {
        case TENSOR_FLOAT32: dtype_str = "float32"; break;
        case TENSOR_INT32: dtype_str = "int32"; break;
        case TENSOR_INT64: dtype_str = "int64"; break;
        case TENSOR_FLOAT64: 
        default: dtype_str = "float64"; break;
    }
    
    lua_pushstring(L, dtype_str);
    return 1;
}

/**
 * @brief tensor:set(i, j, ..., value) - 设置元素值
 */
static int l_tensor_set(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    if (lua_gettop(L) != tensor->ndims + 2) {
        lua_pushnil(L);
        lua_pushfstring(L, "expected %d indices", tensor->ndims);
        return 2;
    }
    
    int64_t indices[TENSOR_MAX_DIMS];
    for (int i = 0; i < tensor->ndims; i++) {
        indices[i] = (int64_t)luaL_checkinteger(L, i + 2);
    }
    
    double value = luaL_checknumber(L, tensor->ndims + 2);
    
    int64_t linear_idx = tensor_multidim_to_linear(tensor, indices);
    if (linear_idx < 0) {
        lua_pushnil(L);
        lua_pushstring(L, "index out of bounds");
        return 2;
    }
    
    tensor_set_value(tensor, linear_idx, value);
    
    lua_pushboolean(L, 1);
    return 1;
}

/**
 * @brief tensor:get(i, j, ...) - 获取元素值
 */
static int l_tensor_get(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    if (lua_gettop(L) != tensor->ndims + 1) {
        lua_pushnil(L);
        lua_pushfstring(L, "expected %d indices", tensor->ndims);
        return 2;
    }
    
    int64_t indices[TENSOR_MAX_DIMS];
    for (int i = 0; i < tensor->ndims; i++) {
        indices[i] = (int64_t)luaL_checkinteger(L, i + 2);
    }
    
    int64_t linear_idx = tensor_multidim_to_linear(tensor, indices);
    if (linear_idx < 0) {
        lua_pushnil(L);
        lua_pushstring(L, "index out of bounds");
        return 2;
    }
    
    double value = tensor_get_value(tensor, linear_idx);
    lua_pushnumber(L, value);
    
    return 1;
}

/**
 * @brief tensor.add(t1, t2) - 加法运算
 */
static int l_tensor_add(lua_State* L) {
    Tensor* t1 = luaL_check_tensor(L, 1);
    Tensor* t2 = luaL_check_tensor(L, 2);
    
    Tensor* result = tensor_elementwise_binary(t1, t2, tensor_op_add);
    if (result == NULL) {
        return luaL_error(L, "broadcast failed");
    }
    
    Tensor* out = tensor_copy_data(result);
    free(result);
    
    lua_newuserdata(L, sizeof(Tensor));
    luaL_getmetatable(L, TENSOR_METATABLE);
    lua_setmetatable(L, -2);
    memcpy(lua_touserdata(L, -1), out, sizeof(Tensor));
    free(out);
    
    return 1;
}

/**
 * @brief tensor.sub(t1, t2) - 减法运算
 */
static int l_tensor_sub(lua_State* L) {
    Tensor* t1 = luaL_check_tensor(L, 1);
    Tensor* t2 = luaL_check_tensor(L, 2);
    
    Tensor* result = tensor_elementwise_binary(t1, t2, tensor_op_sub);
    if (result == NULL) {
        return luaL_error(L, "broadcast failed");
    }
    
    Tensor* out = tensor_copy_data(result);
    free(result);
    
    lua_newuserdata(L, sizeof(Tensor));
    luaL_getmetatable(L, TENSOR_METATABLE);
    lua_setmetatable(L, -2);
    memcpy(lua_touserdata(L, -1), out, sizeof(Tensor));
    free(out);
    
    return 1;
}

/**
 * @brief tensor.mul(t1, t2) - 乘法运算
 */
static int l_tensor_mul(lua_State* L) {
    Tensor* t1 = luaL_check_tensor(L, 1);
    Tensor* t2 = luaL_check_tensor(L, 2);
    
    Tensor* result = tensor_elementwise_binary(t1, t2, tensor_op_mul);
    if (result == NULL) {
        return luaL_error(L, "broadcast failed");
    }
    
    Tensor* out = tensor_copy_data(result);
    free(result);
    
    lua_newuserdata(L, sizeof(Tensor));
    luaL_getmetatable(L, TENSOR_METATABLE);
    lua_setmetatable(L, -2);
    memcpy(lua_touserdata(L, -1), out, sizeof(Tensor));
    free(out);
    
    return 1;
}

/**
 * @brief tensor.div(t1, t2) - 除法运算
 */
static int l_tensor_div(lua_State* L) {
    Tensor* t1 = luaL_check_tensor(L, 1);
    Tensor* t2 = luaL_check_tensor(L, 2);
    
    Tensor* result = tensor_elementwise_binary(t1, t2, tensor_op_div);
    if (result == NULL) {
        return luaL_error(L, "broadcast failed");
    }
    
    Tensor* out = tensor_copy_data(result);
    free(result);
    
    lua_newuserdata(L, sizeof(Tensor));
    luaL_getmetatable(L, TENSOR_METATABLE);
    lua_setmetatable(L, -2);
    memcpy(lua_touserdata(L, -1), out, sizeof(Tensor));
    free(out);
    
    return 1;
}

/**
 * @brief tensor.sum(t[, axis]) - 求和运算
 */
static int l_tensor_sum(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    int axis = -1;
    if (lua_gettop(L) >= 2) {
        axis = (int)luaL_checkinteger(L, 2);
    }
    
    Tensor* result = tensor_reduce(tensor, axis, tensor_reduce_sum);
    
    Tensor* out = tensor_copy_data(result);
    free(result);
    
    lua_newuserdata(L, sizeof(Tensor));
    luaL_getmetatable(L, TENSOR_METATABLE);
    lua_setmetatable(L, -2);
    memcpy(lua_touserdata(L, -1), out, sizeof(Tensor));
    free(out);
    
    return 1;
}

/**
 * @brief tensor.max(t[, axis]) - 求最大值
 */
static int l_tensor_max(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    int axis = -1;
    if (lua_gettop(L) >= 2) {
        axis = (int)luaL_checkinteger(L, 2);
    }
    
    Tensor* result = tensor_reduce(tensor, axis, tensor_reduce_max);
    double max_val = *(double*)result->data;
    free(result);
    
    lua_pushnumber(L, max_val);
    return 1;
}

/**
 * @brief tensor.min(t[, axis]) - 求最小值
 */
static int l_tensor_min(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    int axis = -1;
    if (lua_gettop(L) >= 2) {
        axis = (int)luaL_checkinteger(L, 2);
    }
    
    Tensor* result = tensor_reduce(tensor, axis, tensor_reduce_min);
    double min_val = *(double*)result->data;
    free(result);
    
    lua_pushnumber(L, min_val);
    return 1;
}

/**
 * @brief tensor:transpose() - 转置
 */
static int l_tensor_transpose(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    Tensor* result = (Tensor*)malloc(sizeof(Tensor));
    if (result == NULL) {
        return luaL_error(L, "memory allocation failed");
    }
    
    result->ndims = tensor->ndims;
    result->dtype = tensor->dtype;
    result->itemsize = tensor->itemsize;
    result->size = tensor->size;
    result->owner = 0;
    result->data = tensor->data;
    
    for (int i = 0; i < TENSOR_MAX_DIMS; i++) {
        result->shape[i] = 1;
        result->stride[i] = 0;
    }
    
    for (int i = 0; i < tensor->ndims && i < TENSOR_MAX_DIMS; i++) {
        result->shape[i] = tensor->shape[tensor->ndims - 1 - i];
        result->stride[i] = tensor->stride[tensor->ndims - 1 - i];
    }
    
    lua_newuserdata(L, sizeof(Tensor));
    luaL_getmetatable(L, TENSOR_METATABLE);
    lua_setmetatable(L, -2);
    memcpy(lua_touserdata(L, -1), result, sizeof(Tensor));
    free(result);
    
    return 1;
}

/**
 * @brief tensor:reshape(new_shape) - 重塑
 */
static int l_tensor_reshape(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    if (!lua_istable(L, 2)) {
        return luaL_error(L, "expected table as new shape");
    }
    
    int new_ndims = luaL_len(L, 2);
    if (new_ndims > TENSOR_MAX_DIMS) {
        return luaL_error(L, "too many dimensions");
    }
    
    int64_t new_shape[TENSOR_MAX_DIMS];
    int64_t total = 1;
    for (int i = 0; i < new_ndims; i++) {
        lua_geti(L, 2, i + 1);
        new_shape[i] = (int64_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        
        if (new_shape[i] < 0) {
            return luaL_error(L, "shape dimension must be non-negative");
        }
        total *= new_shape[i];
    }
    
    if (total != tensor->size) {
        return luaL_error(L, "new shape must have same total size");
    }
    
    Tensor* result = (Tensor*)malloc(sizeof(Tensor));
    if (result == NULL) {
        return luaL_error(L, "memory allocation failed");
    }
    
    result->ndims = new_ndims;
    result->dtype = tensor->dtype;
    result->itemsize = tensor->itemsize;
    result->size = tensor->size;
    result->owner = 0;
    result->data = tensor->data;
    
    for (int i = 0; i < TENSOR_MAX_DIMS; i++) {
        result->shape[i] = 1;
    }
    
    for (int i = 0; i < new_ndims && i < TENSOR_MAX_DIMS; i++) {
        result->shape[i] = new_shape[i];
    }
    
    tensor_init_strides(result);
    
    lua_newuserdata(L, sizeof(Tensor));
    luaL_getmetatable(L, TENSOR_METATABLE);
    lua_setmetatable(L, -2);
    memcpy(lua_touserdata(L, -1), result, sizeof(Tensor));
    free(result);
    
    return 1;
}

/**
 * @brief tensor:tolist() - 转换为Lua表
 */
static int l_tensor_tolist(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    if (tensor->ndims == 1) {
        lua_createtable(L, tensor->shape[0], 0);
        for (int64_t i = 0; i < tensor->shape[0]; i++) {
            double* ptr = (double*)tensor_get_element_ptr(tensor, i);
            lua_pushnumber(L, *ptr);
            lua_seti(L, -2, i + 1);
        }
    } else if (tensor->ndims == 2) {
        lua_createtable(L, tensor->shape[0], 0);
        for (int64_t i = 0; i < tensor->shape[0]; i++) {
            lua_createtable(L, tensor->shape[1], 0);
            for (int64_t j = 0; j < tensor->shape[1]; j++) {
                double* ptr = (double*)tensor_get_element_ptr(tensor, 
                                                              i * tensor->shape[1] + j);
                lua_pushnumber(L, *ptr);
                lua_seti(L, -2, j + 1);
            }
            lua_seti(L, -2, i + 1);
        }
    } else {
        lua_pushnil(L);
    }
    
    return 1;
}

/**
 * @brief tensor:clone() - 复制张量
 */
static int l_tensor_clone(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    Tensor* result = tensor_copy_data(tensor);
    if (result == NULL) {
        return luaL_error(L, "memory allocation failed");
    }
    
    lua_newuserdata(L, sizeof(Tensor));
    luaL_getmetatable(L, TENSOR_METATABLE);
    lua_setmetatable(L, -2);
    memcpy(lua_touserdata(L, -1), result, sizeof(Tensor));
    free(result);
    
    return 1;
}

/**
 * @brief tensor:data() - 获取底层一维数据表
 */
static int l_tensor_data(lua_State* L) {
    Tensor* tensor = luaL_check_tensor(L, 1);
    
    lua_createtable(L, tensor->size, 0);
    
    for (int64_t i = 0; i < tensor->size; i++) {
        void* ptr = tensor_get_element_ptr(tensor, i);
        switch (tensor->dtype) {
            case TENSOR_FLOAT64: {
                double val = *(double*)ptr;
                lua_pushnumber(L, val);
                break;
            }
            case TENSOR_FLOAT32: {
                float val = *(float*)ptr;
                lua_pushnumber(L, val);
                break;
            }
            case TENSOR_INT64: {
                int64_t val = *(int64_t*)ptr;
                lua_pushinteger(L, val);
                break;
            }
            case TENSOR_INT32: {
                int32_t val = *(int32_t*)ptr;
                lua_pushinteger(L, val);
                break;
            }
            default:
                lua_pushnil(L);
                break;
        }
        lua_seti(L, -2, i + 1);
    }
    
    return 1;
}

/**
 * @brief tensor.help([name]) - 显示帮助信息
 */
static int l_tensor_help(lua_State* L) {
    const char* func_name = NULL;
    int top = lua_gettop(L);
    
    if (top >= 1 && !lua_isnil(L, 1)) {
        func_name = luaL_checkstring(L, 1);
    }
    
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    
    if (func_name == NULL || strcmp(func_name, "all") == 0) {
        luaL_addstring(&b, "\n========================================\n");
        luaL_addstring(&b, "       Lua张量模块 (tensor) 帮助文档\n");
        luaL_addstring(&b, "========================================\n\n");
        
        luaL_addstring(&b, "一、模块概述\n");
        luaL_addstring(&b, "----------------------------------------\n");
        luaL_addstring(&b, "tensor模块是一个用于创建和操作多维数组（张量）的Lua模块。\n");
        luaL_addstring(&b, "支持多种数据类型、丰富的数学运算和变换操作。\n\n");
        
        luaL_addstring(&b, "二、函数分类\n");
        luaL_addstring(&b, "----------------------------------------\n");
        luaL_addstring(&b, "1. 创建函数: new, zeros, eye\n");
        luaL_addstring(&b, "2. 属性查询: shape, ndims, size, dtype, data\n");
        luaL_addstring(&b, "3. 元素访问: get, set\n");
        luaL_addstring(&b, "4. 数学运算: add, sub, mul, div\n");
        luaL_addstring(&b, "5. 归约运算: sum, max, min\n");
        luaL_addstring(&b, "6. 变换操作: transpose, reshape, clone, tolist\n");
        luaL_addstring(&b, "7. 工具函数: help\n\n");
        
        luaL_addstring(&b, "三、核心概念\n");
        luaL_addstring(&b, "----------------------------------------\n");
        luaL_addstring(&b, "1. 形状(shape): 张量各维度的大小，如 {3, 4} 表示3行4列\n");
        luaL_addstring(&b, "2. 索引: 从1开始，支持负数索引（从末尾计算）\n");
        luaL_addstring(&b, "3. 广播: 不同形状的张量可自动扩展后进行运算\n");
        luaL_addstring(&b, "4. 数据类型: float64(默认), float32, int32, int64\n\n");
        
        luaL_addstring(&b, "四、详细函数说明\n");
        luaL_addstring(&b, "----------------------------------------\n\n");
        
        luaL_addstring(&b, "[new(shape, dtype)]\n");
        luaL_addstring(&b, "  功能: 创建指定形状的张量\n");
        luaL_addstring(&b, "  示例: local t = tensor.new({3, 4}, 'float64')\n\n");
        
        luaL_addstring(&b, "[zeros(shape)]\n");
        luaL_addstring(&b, "  功能: 创建零张量\n");
        luaL_addstring(&b, "  示例: local z = tensor.zeros({2, 2})\n\n");
        
        luaL_addstring(&b, "[eye(n, m, k)]\n");
        luaL_addstring(&b, "  功能: 创建单位矩阵\n");
        luaL_addstring(&b, "  示例: local I = tensor.eye(3) 或 tensor.eye(3, 4, 1)\n\n");
        
        luaL_addstring(&b, "[shape()]\n");
        luaL_addstring(&b, "  功能: 获取张量形状\n");
        luaL_addstring(&b, "  示例: print(t:shape())  -- 返回 {3, 4}\n\n");
        
        luaL_addstring(&b, "[ndims()]\n");
        luaL_addstring(&b, "  功能: 获取维度数\n");
        luaL_addstring(&b, "  示例: print(t:ndims())  -- 返回 2\n\n");
        
        luaL_addstring(&b, "[size()]\n");
        luaL_addstring(&b, "  功能: 获取元素总数\n");
        luaL_addstring(&b, "  示例: print(t:size())  -- 返回 12\n\n");
        
        luaL_addstring(&b, "[dtype()]\n");
        luaL_addstring(&b, "  功能: 获取数据类型\n");
        luaL_addstring(&b, "  示例: print(t:dtype())  -- 返回 'float64'\n\n");
        
        luaL_addstring(&b, "[data()]\n");
        luaL_addstring(&b, "  功能: 获取底层一维数据表\n");
        luaL_addstring(&b, "  示例: local d = t:data()  -- 返回一维表\n\n");
        
        luaL_addstring(&b, "[get(i, j, ...)]\n");
        luaL_addstring(&b, "  功能: 获取元素值\n");
        luaL_addstring(&b, "  示例: print(t:get(1, 1))\n\n");
        
        luaL_addstring(&b, "[set(i, j, ..., value)]\n");
        luaL_addstring(&b, "  功能: 设置元素值\n");
        luaL_addstring(&b, "  示例: t:set(1, 1, 5.0)\n\n");
        
        luaL_addstring(&b, "[add(t1, t2)][sub(t1, t2)][mul(t1, t2)][div(t1, t2)]\n");
        luaL_addstring(&b, "  功能: 逐元素数学运算\n");
        luaL_addstring(&b, "  示例: local r = tensor.add(t1, t2)\n\n");
        
        luaL_addstring(&b, "[sum(t, axis)][max(t, axis)][min(t, axis)]\n");
        luaL_addstring(&b, "  功能: 归约运算\n");
        luaL_addstring(&b, "  示例: tensor.sum(t) 或 tensor.sum(t, 0)\n\n");
        
        luaL_addstring(&b, "[transpose()]\n");
        luaL_addstring(&b, "  功能: 张量转置\n");
        luaL_addstring(&b, "  示例: local t2 = t:transpose()\n\n");
        
        luaL_addstring(&b, "[reshape(shape)]\n");
        luaL_addstring(&b, "  功能: 重塑张量形状\n");
        luaL_addstring(&b, "  示例: local t2 = t:reshape({2, 6})\n\n");
        
        luaL_addstring(&b, "[clone()]\n");
        luaL_addstring(&b, "  功能: 复制张量\n");
        luaL_addstring(&b, "  示例: local t2 = t:clone()\n\n");
        
        luaL_addstring(&b, "[tolist()]\n");
        luaL_addstring(&b, "  功能: 转换为Lua表\n");
        luaL_addstring(&b, "  示例: local list = t:tolist()\n\n");
        
        luaL_addstring(&b, "[help(name)]\n");
        luaL_addstring(&b, "  功能: 显示帮助信息\n");
        luaL_addstring(&b, "  示例: tensor.help() 或 tensor.help('new')\n\n");
        
        luaL_addstring(&b, "========================================\n");
        luaL_addstring(&b, "    输入 tensor.help('函数名') 查看详情\n");
        luaL_addstring(&b, "========================================\n");
    } else {
        luaL_addstring(&b, "\n[");
        luaL_addstring(&b, func_name);
        luaL_addstring(&b, "]\n");
        luaL_addstring(&b, "----------------------------------------\n");
        
        if (strcmp(func_name, "new") == 0) {
            luaL_addstring(&b, "函数签名: tensor.new(shape[, dtype])\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  创建指定形状的张量，所有元素初始化为0。\n\n");
            luaL_addstring(&b, "参数说明:\n");
            luaL_addstring(&b, "  - shape: table类型，包含各维度大小\n");
            luaL_addstring(&b, "  - dtype: 可选，'float64'(默认), 'float32', 'int32', 'int64'\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local t = tensor.new({3, 4})\n");
            luaL_addstring(&b, "  local t2 = tensor.new({2, 3}, 'float32')\n\n");
        } else if (strcmp(func_name, "zeros") == 0) {
            luaL_addstring(&b, "函数签名: tensor.zeros(shape)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  创建指定形状的零张量。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local z = tensor.zeros({3, 3})\n\n");
        } else if (strcmp(func_name, "eye") == 0) {
            luaL_addstring(&b, "函数签名: tensor.eye(n[, m[, k]])\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  创建单位矩阵或对角矩阵。\n\n");
            luaL_addstring(&b, "参数说明:\n");
            luaL_addstring(&b, "  - n: 行数\n");
            luaL_addstring(&b, "  - m: 列数(默认=n)\n");
            luaL_addstring(&b, "  - k: 对角线偏移\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local I = tensor.eye(3)\n");
            luaL_addstring(&b, "  local I2 = tensor.eye(3, 4, 1)\n\n");
        } else if (strcmp(func_name, "shape") == 0) {
            luaL_addstring(&b, "函数签名: tensor:shape()\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  获取张量的形状。\n\n");
            luaL_addstring(&b, "返回值: table类型\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  print(t:shape())\n\n");
        } else if (strcmp(func_name, "ndims") == 0) {
            luaL_addstring(&b, "函数签名: tensor:ndims()\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  获取张量的维度数量。\n\n");
            luaL_addstring(&b, "返回值: 整数\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  print(t:ndims())\n\n");
        } else if (strcmp(func_name, "size") == 0) {
            luaL_addstring(&b, "函数签名: tensor:size()\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  获取张量元素总数。\n\n");
            luaL_addstring(&b, "返回值: 整数\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  print(t:size())\n\n");
        } else if (strcmp(func_name, "dtype") == 0) {
            luaL_addstring(&b, "函数签名: tensor:dtype()\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  获取张量的数据类型。\n\n");
            luaL_addstring(&b, "返回值: 字符串\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  print(t:dtype())\n\n");
        } else if (strcmp(func_name, "data") == 0) {
            luaL_addstring(&b, "函数签名: tensor:data()\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  获取底层一维数据表。\n\n");
            luaL_addstring(&b, "返回值: 一维Lua表\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local d = t:data()\n\n");
        } else if (strcmp(func_name, "get") == 0) {
            luaL_addstring(&b, "函数签名: tensor:get(i, j, ...)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  获取张量中指定位置的元素值。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  print(t:get(1, 1))\n\n");
        } else if (strcmp(func_name, "set") == 0) {
            luaL_addstring(&b, "函数签名: tensor:set(i, j, ..., value)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  设置张量中指定位置的元素值。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  t:set(1, 1, 5.0)\n\n");
        } else if (strcmp(func_name, "add") == 0) {
            luaL_addstring(&b, "函数签名: tensor.add(t1, t2)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  两个张量逐元素相加。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local r = tensor.add(t1, t2)\n\n");
        } else if (strcmp(func_name, "sub") == 0) {
            luaL_addstring(&b, "函数签名: tensor.sub(t1, t2)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  两个张量逐元素相减。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local r = tensor.sub(t1, t2)\n\n");
        } else if (strcmp(func_name, "mul") == 0) {
            luaL_addstring(&b, "函数签名: tensor.mul(t1, t2)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  两个张量逐元素相乘。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local r = tensor.mul(t1, t2)\n\n");
        } else if (strcmp(func_name, "div") == 0) {
            luaL_addstring(&b, "函数签名: tensor.div(t1, t2)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  两个张量逐元素相除。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local r = tensor.div(t1, t2)\n\n");
        } else if (strcmp(func_name, "sum") == 0) {
            luaL_addstring(&b, "函数签名: tensor.sum(t[, axis])\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  对张量元素求和。\n\n");
            luaL_addstring(&b, "参数说明:\n");
            luaL_addstring(&b, "  - axis: 可选，归约维度\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  tensor.sum(t)      -- 所有元素求和\n");
            luaL_addstring(&b, "  tensor.sum(t, 0)   -- 按列求和\n\n");
        } else if (strcmp(func_name, "max") == 0) {
            luaL_addstring(&b, "函数签名: tensor.max(t[, axis])\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  获取张量中的最大值。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  tensor.max(t)\n\n");
        } else if (strcmp(func_name, "min") == 0) {
            luaL_addstring(&b, "函数签名: tensor.min(t[, axis])\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  获取张量中的最小值。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  tensor.min(t)\n\n");
        } else if (strcmp(func_name, "transpose") == 0) {
            luaL_addstring(&b, "函数签名: tensor:transpose()\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  交换张量的所有维度。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local t2 = t:transpose()\n\n");
        } else if (strcmp(func_name, "reshape") == 0) {
            luaL_addstring(&b, "函数签名: tensor:reshape(new_shape)\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  改变张量的形状。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local t2 = t:reshape({2, 3})\n\n");
        } else if (strcmp(func_name, "clone") == 0) {
            luaL_addstring(&b, "函数签名: tensor:clone()\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  创建张量的深拷贝。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local t2 = t:clone()\n\n");
        } else if (strcmp(func_name, "tolist") == 0) {
            luaL_addstring(&b, "函数签名: tensor:tolist()\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  将张量转换为Lua嵌套表。\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  local list = t:tolist()\n\n");
        } else if (strcmp(func_name, "help") == 0) {
            luaL_addstring(&b, "函数签名: tensor.help([name])\n\n");
            luaL_addstring(&b, "功能描述:\n");
            luaL_addstring(&b, "  显示帮助信息。\n\n");
            luaL_addstring(&b, "参数说明:\n");
            luaL_addstring(&b, "  - name: 可选，函数名\n\n");
            luaL_addstring(&b, "使用示例:\n");
            luaL_addstring(&b, "  tensor.help()       -- 显示所有函数帮助\n");
            luaL_addstring(&b, "  tensor.help('new')  -- 显示new函数帮助\n\n");
        } else {
            luaL_addstring(&b, "\n[错误]未找到函数 '");
            luaL_addstring(&b, func_name);
            luaL_addstring(&b, "'\n\n");
            luaL_addstring(&b, "可用函数: new, zeros, eye, shape, ndims, size, dtype, data\n");
            luaL_addstring(&b, "          get, set, add, sub, mul, div\n");
            luaL_addstring(&b, "          sum, max, min, transpose, reshape, clone, tolist, help\n\n");
        }
    }
    
    luaL_pushresult(&b);
    return 1;
}

/**
 * @brief 张量元方法：gc
 */
static int l_tensor_gc(lua_State* L) {
    Tensor* tensor = (Tensor*)luaL_checkudata(L, 1, TENSOR_METATABLE);
    tensor_free(tensor);
    return 0;
}

/**
 * @brief 张量元方法：tostring
 */
static int l_tensor_tostring(lua_State* L) {
    Tensor* tensor = (Tensor*)luaL_checkudata(L, 1, TENSOR_METATABLE);
    
    lua_pushfstring(L, "Tensor(shape=[%lld", tensor->shape[0]);
    for (int i = 1; i < tensor->ndims; i++) {
        lua_pushfstring(L, ", %lld", tensor->shape[i]);
    }
    lua_pushfstring(L, "], size=%lld, dtype=%s)", 
                    tensor->size,
                    tensor->dtype == TENSOR_FLOAT64 ? "float64" : 
                    tensor->dtype == TENSOR_FLOAT32 ? "float32" : 
                    tensor->dtype == TENSOR_INT64 ? "int64" : "int32");
    
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
    lua_pushliteral(L, "Tensor module for Lua - 基于C实现的高性能多维数组");
    lua_settable(L, -3);
    
    lua_pushliteral(L, "_VERSION");
    lua_pushliteral(L, "2.0");
    lua_settable(L, -3);
}

/**
 * @brief 模块注册表
 */
static const struct luaL_Reg tensorlib[] = {
    {"new", l_tensor_new},
    {"zeros", l_tensor_zeros},
    {"eye", l_tensor_eye},
    {"shape", l_tensor_shape},
    {"ndims", l_tensor_ndims},
    {"size", l_tensor_size},
    {"dtype", l_tensor_dtype},
    {"set", l_tensor_set},
    {"get", l_tensor_get},
    {"add", l_tensor_add},
    {"sub", l_tensor_sub},
    {"mul", l_tensor_mul},
    {"div", l_tensor_div},
    {"sum", l_tensor_sum},
    {"max", l_tensor_max},
    {"min", l_tensor_min},
    {"transpose", l_tensor_transpose},
    {"reshape", l_tensor_reshape},
    {"tolist", l_tensor_tolist},
    {"clone", l_tensor_clone},
    {"data", l_tensor_data},
    {"help", l_tensor_help},
    {NULL, NULL}
};

/**
 * @brief 张量元方法表
 */
static const struct luaL_Reg tensor_meta[] = {
    {"__gc", l_tensor_gc},
    {"__tostring", l_tensor_tostring},
    {"shape", l_tensor_shape},
    {"ndims", l_tensor_ndims},
    {"size", l_tensor_size},
    {"dtype", l_tensor_dtype},
    {"set", l_tensor_set},
    {"get", l_tensor_get},
    {"sum", l_tensor_sum},
    {"max", l_tensor_max},
    {"min", l_tensor_min},
    {"transpose", l_tensor_transpose},
    {"reshape", l_tensor_reshape},
    {"tolist", l_tensor_tolist},
    {"clone", l_tensor_clone},
    {"data", l_tensor_data},
    {NULL, NULL}
};

/**
 * @brief 模块初始化函数
 */
int luaopen_tensor(lua_State* L) {
    luaL_newmetatable(L, TENSOR_METATABLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, tensor_meta, 0);
    
    luaL_newlib(L, tensorlib);
    set_info(L);
    
    return 1;
}
