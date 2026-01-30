/**
 * @file lief_elf_wrapper.hpp
 * @brief LIEF ELF C++ API的C语言wrapper
 * @description 为LIEF的C++ API提供C语言接口，用于Lua绑定
 * @author DifierLine
 * @date 2026-01-30
 */

#ifndef LIEF_ELF_WRAPPER_HPP
#define LIEF_ELF_WRAPPER_HPP

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 前向声明 ========== */
typedef struct Elf_Binary_Wrapper Elf_Binary_Wrapper;

/* ========== 二进制文件操作 ========== */

/**
 * @brief 解析ELF文件
 * @param filepath 文件路径
 * @return 成功返回Binary wrapper指针，失败返回NULL
 */
Elf_Binary_Wrapper* lief_elf_parse(const char* filepath);

/**
 * @brief 从内存解析ELF
 * @param data 数据指针
 * @param size 数据大小
 * @return 成功返回Binary wrapper指针，失败返回NULL
 */
Elf_Binary_Wrapper* lief_elf_parse_from_memory(const uint8_t* data, size_t size);

/**
 * @brief 销毁Binary wrapper
 * @param wrapper Binary wrapper指针
 */
void lief_elf_destroy(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 写入ELF到文件
 * @param wrapper Binary wrapper指针
 * @param filepath 输出文件路径
 * @return 成功返回0，失败返回-1
 */
int lief_elf_write(Elf_Binary_Wrapper* wrapper, const char* filepath);

/**
 * @brief 获取ELF原始数据
 * @param wrapper Binary wrapper指针
 * @param out_size 输出数据大小
 * @return 返回数据指针，需要调用者释放
 */
uint8_t* lief_elf_raw(Elf_Binary_Wrapper* wrapper, size_t* out_size);

/* ========== Header操作 ========== */

/**
 * @brief 获取入口点
 * @param wrapper Binary wrapper指针
 * @return 入口点地址
 */
uint64_t lief_elf_get_entrypoint(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 设置入口点
 * @param wrapper Binary wrapper指针
 * @param entrypoint 新的入口点地址
 */
void lief_elf_set_entrypoint(Elf_Binary_Wrapper* wrapper, uint64_t entrypoint);

/**
 * @brief 获取文件类型
 * @param wrapper Binary wrapper指针
 * @return 文件类型 (ET_NONE, ET_REL, ET_EXEC, ET_DYN, ET_CORE)
 */
uint32_t lief_elf_get_type(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 设置文件类型
 * @param wrapper Binary wrapper指针
 * @param type 新的文件类型
 */
void lief_elf_set_type(Elf_Binary_Wrapper* wrapper, uint32_t type);

/**
 * @brief 获取机器架构
 * @param wrapper Binary wrapper指针
 * @return 机器架构类型
 */
uint32_t lief_elf_get_machine(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 设置机器架构
 * @param wrapper Binary wrapper指针
 * @param machine 新的机器架构类型
 */
void lief_elf_set_machine(Elf_Binary_Wrapper* wrapper, uint32_t machine);

/* ========== 解释器操作 ========== */

/**
 * @brief 获取解释器路径
 * @param wrapper Binary wrapper指针
 * @return 解释器路径字符串
 */
const char* lief_elf_get_interpreter(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 设置解释器路径
 * @param wrapper Binary wrapper指针
 * @param interpreter 新的解释器路径
 */
void lief_elf_set_interpreter(Elf_Binary_Wrapper* wrapper, const char* interpreter);

/**
 * @brief 检查是否有解释器
 * @param wrapper Binary wrapper指针
 * @return 有返回1，无返回0
 */
int lief_elf_has_interpreter(Elf_Binary_Wrapper* wrapper);

/* ========== Section操作 ========== */

/**
 * @brief 获取节数量
 * @param wrapper Binary wrapper指针
 * @return 节数量
 */
size_t lief_elf_sections_count(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 通过索引获取节名称
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @return 节名称
 */
const char* lief_elf_section_name(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取节虚拟地址
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @return 虚拟地址
 */
uint64_t lief_elf_section_virtual_address(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取节大小
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @return 节大小
 */
uint64_t lief_elf_section_size(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取节偏移
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @return 文件偏移
 */
uint64_t lief_elf_section_offset(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取节类型
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @return 节类型
 */
uint32_t lief_elf_section_type(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取节标志
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @return 节标志
 */
uint64_t lief_elf_section_flags(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取节内容
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @param out_size 输出内容大小
 * @return 内容指针（不需要释放）
 */
const uint8_t* lief_elf_section_content(Elf_Binary_Wrapper* wrapper, size_t index, size_t* out_size);

/**
 * @brief 设置节内容
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @param content 新内容
 * @param size 内容大小
 * @return 成功返回0，失败返回-1
 */
int lief_elf_section_set_content(Elf_Binary_Wrapper* wrapper, size_t index, const uint8_t* content, size_t size);

/**
 * @brief 添加新节
 * @param wrapper Binary wrapper指针
 * @param name 节名称
 * @param type 节类型
 * @param flags 节标志
 * @param content 节内容
 * @param size 内容大小
 * @param loaded 是否加载到内存
 * @return 成功返回新节索引，失败返回-1
 */
int lief_elf_add_section(Elf_Binary_Wrapper* wrapper, const char* name, 
                         uint32_t type, uint64_t flags,
                         const uint8_t* content, size_t size, int loaded);

/**
 * @brief 移除节
 * @param wrapper Binary wrapper指针
 * @param name 节名称
 * @param clear 是否清零原内容
 * @return 成功返回0，失败返回-1
 */
int lief_elf_remove_section(Elf_Binary_Wrapper* wrapper, const char* name, int clear);

/* ========== Segment操作 ========== */

/**
 * @brief 获取段数量
 * @param wrapper Binary wrapper指针
 * @return 段数量
 */
size_t lief_elf_segments_count(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 通过索引获取段类型
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @return 段类型
 */
uint32_t lief_elf_segment_type(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取段标志
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @return 段标志
 */
uint32_t lief_elf_segment_flags(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取段虚拟地址
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @return 虚拟地址
 */
uint64_t lief_elf_segment_virtual_address(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取段虚拟大小
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @return 虚拟大小
 */
uint64_t lief_elf_segment_virtual_size(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取段文件偏移
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @return 文件偏移
 */
uint64_t lief_elf_segment_offset(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取段文件大小
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @return 文件大小
 */
uint64_t lief_elf_segment_file_size(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取段内容
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @param out_size 输出内容大小
 * @return 内容指针（不需要释放）
 */
const uint8_t* lief_elf_segment_content(Elf_Binary_Wrapper* wrapper, size_t index, size_t* out_size);

/**
 * @brief 设置段内容
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @param content 新内容
 * @param size 内容大小
 * @return 成功返回0，失败返回-1
 */
int lief_elf_segment_set_content(Elf_Binary_Wrapper* wrapper, size_t index, const uint8_t* content, size_t size);

/**
 * @brief 添加新段
 * @param wrapper Binary wrapper指针
 * @param type 段类型
 * @param flags 段标志
 * @param content 段内容
 * @param size 内容大小
 * @param alignment 对齐
 * @return 成功返回新段索引，失败返回-1
 */
int lief_elf_add_segment(Elf_Binary_Wrapper* wrapper, uint32_t type, uint32_t flags,
                         const uint8_t* content, size_t size, uint64_t alignment);

/* ========== 符号操作 ========== */

/**
 * @brief 获取动态符号数量
 * @param wrapper Binary wrapper指针
 * @return 动态符号数量
 */
size_t lief_elf_dynamic_symbols_count(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 获取symtab符号数量
 * @param wrapper Binary wrapper指针
 * @return symtab符号数量
 */
size_t lief_elf_symtab_symbols_count(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 通过索引获取动态符号名称
 * @param wrapper Binary wrapper指针
 * @param index 符号索引
 * @return 符号名称
 */
const char* lief_elf_dynamic_symbol_name(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取动态符号值
 * @param wrapper Binary wrapper指针
 * @param index 符号索引
 * @return 符号值
 */
uint64_t lief_elf_dynamic_symbol_value(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取动态符号大小
 * @param wrapper Binary wrapper指针
 * @param index 符号索引
 * @return 符号大小
 */
uint64_t lief_elf_dynamic_symbol_size(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取动态符号类型
 * @param wrapper Binary wrapper指针
 * @param index 符号索引
 * @return 符号类型
 */
uint32_t lief_elf_dynamic_symbol_type(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取动态符号绑定类型
 * @param wrapper Binary wrapper指针
 * @param index 符号索引
 * @return 绑定类型
 */
uint32_t lief_elf_dynamic_symbol_binding(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 添加动态符号
 * @param wrapper Binary wrapper指针
 * @param name 符号名称
 * @param value 符号值
 * @param size 符号大小
 * @param type 符号类型
 * @param binding 绑定类型
 * @return 成功返回新符号索引，失败返回-1
 */
int lief_elf_add_dynamic_symbol(Elf_Binary_Wrapper* wrapper, const char* name,
                                uint64_t value, uint64_t size, 
                                uint32_t type, uint32_t binding);

/**
 * @brief 移除动态符号
 * @param wrapper Binary wrapper指针
 * @param name 符号名称
 * @return 成功返回0，失败返回-1
 */
int lief_elf_remove_dynamic_symbol(Elf_Binary_Wrapper* wrapper, const char* name);

/**
 * @brief 导出符号
 * @param wrapper Binary wrapper指针
 * @param name 符号名称
 * @param value 符号值
 * @return 成功返回0，失败返回-1
 */
int lief_elf_export_symbol(Elf_Binary_Wrapper* wrapper, const char* name, uint64_t value);

/* ========== 动态条目操作 ========== */

/**
 * @brief 获取动态条目数量
 * @param wrapper Binary wrapper指针
 * @return 动态条目数量
 */
size_t lief_elf_dynamic_entries_count(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 通过索引获取动态条目标签
 * @param wrapper Binary wrapper指针
 * @param index 条目索引
 * @return 标签值
 */
uint64_t lief_elf_dynamic_entry_tag(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取动态条目值
 * @param wrapper Binary wrapper指针
 * @param index 条目索引
 * @return 值
 */
uint64_t lief_elf_dynamic_entry_value(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 移除指定标签的动态条目
 * @param wrapper Binary wrapper指针
 * @param tag 要移除的标签
 * @return 成功返回0，失败返回-1
 */
int lief_elf_remove_dynamic_entry(Elf_Binary_Wrapper* wrapper, uint64_t tag);

/* ========== 库依赖操作 ========== */

/**
 * @brief 添加库依赖
 * @param wrapper Binary wrapper指针
 * @param library_name 库名称
 * @return 成功返回0，失败返回-1
 */
int lief_elf_add_library(Elf_Binary_Wrapper* wrapper, const char* library_name);

/**
 * @brief 移除库依赖
 * @param wrapper Binary wrapper指针
 * @param library_name 库名称
 * @return 成功返回0，失败返回-1
 */
int lief_elf_remove_library(Elf_Binary_Wrapper* wrapper, const char* library_name);

/**
 * @brief 检查是否有指定库依赖
 * @param wrapper Binary wrapper指针
 * @param library_name 库名称
 * @return 有返回1，无返回0
 */
int lief_elf_has_library(Elf_Binary_Wrapper* wrapper, const char* library_name);

/* ========== Patch操作 ========== */

/**
 * @brief 在虚拟地址处打补丁（字节数组）
 * @param wrapper Binary wrapper指针
 * @param address 虚拟地址
 * @param patch 补丁数据
 * @param size 补丁大小
 * @return 成功返回0，失败返回-1
 */
int lief_elf_patch_address(Elf_Binary_Wrapper* wrapper, uint64_t address, 
                           const uint8_t* patch, size_t size);

/**
 * @brief 在虚拟地址处打补丁（整数值）
 * @param wrapper Binary wrapper指针
 * @param address 虚拟地址
 * @param value 要写入的值
 * @param size 值的字节大小(1,2,4,8)
 * @return 成功返回0，失败返回-1
 */
int lief_elf_patch_address_value(Elf_Binary_Wrapper* wrapper, uint64_t address, 
                                 uint64_t value, size_t size);

/**
 * @brief 修补PLT/GOT
 * @param wrapper Binary wrapper指针
 * @param symbol_name 符号名称
 * @param address 新地址
 * @return 成功返回0，失败返回-1
 */
int lief_elf_patch_pltgot(Elf_Binary_Wrapper* wrapper, const char* symbol_name, uint64_t address);

/* ========== 其他功能 ========== */

/**
 * @brief Strip二进制（移除符号表）
 * @param wrapper Binary wrapper指针
 */
void lief_elf_strip(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 检查是否为PIE
 * @param wrapper Binary wrapper指针
 * @return 是PIE返回1，否则返回0
 */
int lief_elf_is_pie(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 检查是否有NX保护
 * @param wrapper Binary wrapper指针
 * @return 有NX返回1，否则返回0
 */
int lief_elf_has_nx(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 获取image base
 * @param wrapper Binary wrapper指针
 * @return image base地址
 */
uint64_t lief_elf_imagebase(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 虚拟地址转文件偏移
 * @param wrapper Binary wrapper指针
 * @param virtual_address 虚拟地址
 * @param out_offset 输出偏移
 * @return 成功返回0，失败返回-1
 */
int lief_elf_va_to_offset(Elf_Binary_Wrapper* wrapper, uint64_t virtual_address, uint64_t* out_offset);

/**
 * @brief 文件偏移转虚拟地址
 * @param wrapper Binary wrapper指针
 * @param offset 文件偏移
 * @param out_va 输出虚拟地址
 * @return 成功返回0，失败返回-1
 */
int lief_elf_offset_to_va(Elf_Binary_Wrapper* wrapper, uint64_t offset, uint64_t* out_va);

/**
 * @brief 检查文件是否为ELF格式
 * @param filepath 文件路径
 * @return 是ELF返回1，否则返回0
 */
int lief_is_elf(const char* filepath);

/* ========== 重定位操作 ========== */

/**
 * @brief 获取重定位数量
 * @param wrapper Binary wrapper指针
 * @return 重定位数量
 */
size_t lief_elf_relocations_count(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 通过索引获取重定位地址
 * @param wrapper Binary wrapper指针
 * @param index 重定位索引
 * @return 地址
 */
uint64_t lief_elf_relocation_address(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取重定位类型
 * @param wrapper Binary wrapper指针
 * @param index 重定位索引
 * @return 类型
 */
uint32_t lief_elf_relocation_type(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取重定位addend
 * @param wrapper Binary wrapper指针
 * @param index 重定位索引
 * @return addend值
 */
int64_t lief_elf_relocation_addend(Elf_Binary_Wrapper* wrapper, size_t index);

/* ========== 扩展Section操作 ========== */

/**
 * @brief 获取节对齐
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @return 对齐值
 */
uint64_t lief_elf_section_alignment(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 设置节对齐
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @param alignment 对齐值
 * @return 成功返回0，失败返回-1
 */
int lief_elf_section_set_alignment(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t alignment);

/**
 * @brief 获取节条目大小
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @return 条目大小
 */
uint64_t lief_elf_section_entry_size(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 设置节条目大小
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @param entry_size 条目大小
 * @return 成功返回0，失败返回-1
 */
int lief_elf_section_set_entry_size(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t entry_size);

/**
 * @brief 获取节信息字段
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @return 信息值
 */
uint32_t lief_elf_section_info(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 设置节信息字段
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @param info 信息值
 * @return 成功返回0，失败返回-1
 */
int lief_elf_section_set_info(Elf_Binary_Wrapper* wrapper, size_t index, uint32_t info);

/**
 * @brief 获取节链接字段
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @return 链接值
 */
uint32_t lief_elf_section_link(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 设置节链接字段
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @param link 链接值
 * @return 成功返回0，失败返回-1
 */
int lief_elf_section_set_link(Elf_Binary_Wrapper* wrapper, size_t index, uint32_t link);

/**
 * @brief 设置节类型
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @param type 类型值
 * @return 成功返回0，失败返回-1
 */
int lief_elf_section_set_type(Elf_Binary_Wrapper* wrapper, size_t index, uint32_t type);

/**
 * @brief 设置节标志
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @param flags 标志值
 * @return 成功返回0，失败返回-1
 */
int lief_elf_section_set_flags(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t flags);

/**
 * @brief 设置节虚拟地址
 * @param wrapper Binary wrapper指针
 * @param index 节索引
 * @param va 虚拟地址
 * @return 成功返回0，失败返回-1
 */
int lief_elf_section_set_virtual_address(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t va);

/**
 * @brief 通过名称获取节索引
 * @param wrapper Binary wrapper指针
 * @param name 节名称
 * @return 成功返回索引，失败返回-1
 */
int lief_elf_get_section_index(Elf_Binary_Wrapper* wrapper, const char* name);

/**
 * @brief 检查是否存在指定名称的节
 * @param wrapper Binary wrapper指针
 * @param name 节名称
 * @return 存在返回1，不存在返回0
 */
int lief_elf_has_section(Elf_Binary_Wrapper* wrapper, const char* name);

/* ========== 扩展Segment操作 ========== */

/**
 * @brief 获取段物理地址
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @return 物理地址
 */
uint64_t lief_elf_segment_physical_address(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 设置段物理地址
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @param paddr 物理地址
 * @return 成功返回0，失败返回-1
 */
int lief_elf_segment_set_physical_address(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t paddr);

/**
 * @brief 获取段对齐
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @return 对齐值
 */
uint64_t lief_elf_segment_alignment(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 设置段对齐
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @param alignment 对齐值
 * @return 成功返回0，失败返回-1
 */
int lief_elf_segment_set_alignment(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t alignment);

/**
 * @brief 设置段类型
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @param type 类型值
 * @return 成功返回0，失败返回-1
 */
int lief_elf_segment_set_type(Elf_Binary_Wrapper* wrapper, size_t index, uint32_t type);

/**
 * @brief 设置段标志
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @param flags 标志值
 * @return 成功返回0，失败返回-1
 */
int lief_elf_segment_set_flags(Elf_Binary_Wrapper* wrapper, size_t index, uint32_t flags);

/**
 * @brief 设置段虚拟地址
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @param va 虚拟地址
 * @return 成功返回0，失败返回-1
 */
int lief_elf_segment_set_virtual_address(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t va);

/**
 * @brief 设置段虚拟大小
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @param size 虚拟大小
 * @return 成功返回0，失败返回-1
 */
int lief_elf_segment_set_virtual_size(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t size);

/**
 * @brief 设置段文件偏移
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @param offset 文件偏移
 * @return 成功返回0，失败返回-1
 */
int lief_elf_segment_set_file_offset(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t offset);

/**
 * @brief 设置段物理大小（文件大小）
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @param size 物理大小
 * @return 成功返回0，失败返回-1
 */
int lief_elf_segment_set_physical_size(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t size);

/**
 * @brief 通过类型获取第一个段索引
 * @param wrapper Binary wrapper指针
 * @param type 段类型
 * @return 成功返回索引，失败返回-1
 */
int lief_elf_get_segment_index(Elf_Binary_Wrapper* wrapper, uint32_t type);

/**
 * @brief 检查是否存在指定类型的段
 * @param wrapper Binary wrapper指针
 * @param type 段类型
 * @return 存在返回1，不存在返回0
 */
int lief_elf_has_segment(Elf_Binary_Wrapper* wrapper, uint32_t type);

/**
 * @brief 移除段
 * @param wrapper Binary wrapper指针
 * @param index 段索引
 * @param clear 是否清零原内容
 * @return 成功返回0，失败返回-1
 */
int lief_elf_remove_segment(Elf_Binary_Wrapper* wrapper, size_t index, int clear);

/* ========== symtab符号操作 ========== */

/**
 * @brief 通过索引获取symtab符号名称
 * @param wrapper Binary wrapper指针
 * @param index 符号索引
 * @return 符号名称
 */
const char* lief_elf_symtab_symbol_name(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取symtab符号值
 * @param wrapper Binary wrapper指针
 * @param index 符号索引
 * @return 符号值
 */
uint64_t lief_elf_symtab_symbol_value(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取symtab符号大小
 * @param wrapper Binary wrapper指针
 * @param index 符号索引
 * @return 符号大小
 */
uint64_t lief_elf_symtab_symbol_size(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取symtab符号类型
 * @param wrapper Binary wrapper指针
 * @param index 符号索引
 * @return 符号类型
 */
uint32_t lief_elf_symtab_symbol_type(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 通过索引获取symtab符号绑定类型
 * @param wrapper Binary wrapper指针
 * @param index 符号索引
 * @return 绑定类型
 */
uint32_t lief_elf_symtab_symbol_binding(Elf_Binary_Wrapper* wrapper, size_t index);

/**
 * @brief 添加symtab符号
 * @param wrapper Binary wrapper指针
 * @param name 符号名称
 * @param value 符号值
 * @param size 符号大小
 * @param type 符号类型
 * @param binding 绑定类型
 * @return 成功返回新符号索引，失败返回-1
 */
int lief_elf_add_symtab_symbol(Elf_Binary_Wrapper* wrapper, const char* name,
                               uint64_t value, uint64_t size,
                               uint32_t type, uint32_t binding);

/**
 * @brief 移除symtab符号
 * @param wrapper Binary wrapper指针
 * @param name 符号名称
 * @return 成功返回0，失败返回-1
 */
int lief_elf_remove_symtab_symbol(Elf_Binary_Wrapper* wrapper, const char* name);

/* ========== 内存内容操作 ========== */

/**
 * @brief 从虚拟地址读取内容
 * @param wrapper Binary wrapper指针
 * @param va 虚拟地址
 * @param size 要读取的大小
 * @param out_size 输出实际读取的大小
 * @return 内容指针（不需要释放）
 */
const uint8_t* lief_elf_get_content_from_va(Elf_Binary_Wrapper* wrapper, uint64_t va, 
                                            uint64_t size, size_t* out_size);

/**
 * @brief 从偏移获取节索引
 * @param wrapper Binary wrapper指针
 * @param offset 文件偏移
 * @return 成功返回索引，失败返回-1
 */
int lief_elf_section_from_offset(Elf_Binary_Wrapper* wrapper, uint64_t offset);

/**
 * @brief 从虚拟地址获取节索引
 * @param wrapper Binary wrapper指针
 * @param va 虚拟地址
 * @return 成功返回索引，失败返回-1
 */
int lief_elf_section_from_va(Elf_Binary_Wrapper* wrapper, uint64_t va);

/**
 * @brief 从偏移获取段索引
 * @param wrapper Binary wrapper指针
 * @param offset 文件偏移
 * @return 成功返回索引，失败返回-1
 */
int lief_elf_segment_from_offset(Elf_Binary_Wrapper* wrapper, uint64_t offset);

/**
 * @brief 从虚拟地址获取段索引
 * @param wrapper Binary wrapper指针
 * @param va 虚拟地址
 * @return 成功返回索引，失败返回-1
 */
int lief_elf_segment_from_va(Elf_Binary_Wrapper* wrapper, uint64_t va);

/* ========== 二进制信息 ========== */

/**
 * @brief 获取虚拟大小（映射后的总大小）
 * @param wrapper Binary wrapper指针
 * @return 虚拟大小
 */
uint64_t lief_elf_virtual_size(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 获取EOF偏移
 * @param wrapper Binary wrapper指针
 * @return EOF偏移
 */
uint64_t lief_elf_eof_offset(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 检查是否针对Android
 * @param wrapper Binary wrapper指针
 * @return 是Android返回1，否则返回0
 */
int lief_elf_is_targeting_android(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 检查是否有overlay数据
 * @param wrapper Binary wrapper指针
 * @return 有返回1，否则返回0
 */
int lief_elf_has_overlay(Elf_Binary_Wrapper* wrapper);

/**
 * @brief 获取overlay数据
 * @param wrapper Binary wrapper指针
 * @param out_size 输出大小
 * @return 数据指针（不需要释放）
 */
const uint8_t* lief_elf_get_overlay(Elf_Binary_Wrapper* wrapper, size_t* out_size);

/**
 * @brief 设置overlay数据
 * @param wrapper Binary wrapper指针
 * @param data 数据指针
 * @param size 数据大小
 * @return 成功返回0，失败返回-1
 */
int lief_elf_set_overlay(Elf_Binary_Wrapper* wrapper, const uint8_t* data, size_t size);

/* ========== 添加动态重定位 ========== */

/**
 * @brief 添加动态重定位
 * @param wrapper Binary wrapper指针
 * @param address 重定位地址
 * @param type 重定位类型
 * @param addend addend值
 * @param symbol_name 符号名称（可为NULL）
 * @return 成功返回0，失败返回-1
 */
int lief_elf_add_dynamic_relocation(Elf_Binary_Wrapper* wrapper, uint64_t address,
                                    uint32_t type, int64_t addend,
                                    const char* symbol_name);

/**
 * @brief 添加PLT/GOT重定位
 * @param wrapper Binary wrapper指针
 * @param address 重定位地址
 * @param type 重定位类型
 * @param symbol_name 符号名称
 * @return 成功返回0，失败返回-1
 */
int lief_elf_add_pltgot_relocation(Elf_Binary_Wrapper* wrapper, uint64_t address,
                                   uint32_t type, const char* symbol_name);

/* ========== 扩展动态条目操作 ========== */

/**
 * @brief 通过标签获取动态条目值
 * @param wrapper Binary wrapper指针
 * @param tag 标签
 * @param out_value 输出值
 * @return 成功返回0，失败返回-1
 */
int lief_elf_get_dynamic_entry_by_tag(Elf_Binary_Wrapper* wrapper, uint64_t tag, uint64_t* out_value);

/**
 * @brief 检查是否存在指定标签的动态条目
 * @param wrapper Binary wrapper指针
 * @param tag 标签
 * @return 存在返回1，不存在返回0
 */
int lief_elf_has_dynamic_entry(Elf_Binary_Wrapper* wrapper, uint64_t tag);

#ifdef __cplusplus
}
#endif

#endif /* LIEF_ELF_WRAPPER_HPP */
