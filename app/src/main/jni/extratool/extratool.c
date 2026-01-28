#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

// ELF文件头结构（64位）
struct elf64_hdr {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed));

// ELF节头结构（64位）
struct elf64_shdr {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} __attribute__((packed));

// ELF常量定义
#define ELFMAG0 0x7F
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define ELFCLASS64 2
#define EM_AARCH64 183  // ARM64 architecture

// 函数声明
bool is_valid_elf(const struct elf64_hdr *hdr);
int32_t parse_arm64_sub_instruction(uint32_t instr);
int search_sub_instructions(const uint8_t *text_data, size_t text_size);
int extract_key_from_so(const char *file_path);

// 函数声明
static int decrypt_single(lua_State *L, const char *in_path, const char *out_path, int key, int analyze);
static int check_registration(lua_State *L);
static void encrypt_data(char *data, size_t data_len, const char *key);
static void decrypt_data(char *data, size_t data_len, const char *key);
static char* reverse_string(char *str);
static int parse_date(const char *date_str, int *year, int *month, int *day);
static int compare_dates(const char *date1, const char *date2);

/**
 * 验证是否为有效的ELF文件
 */
bool is_valid_elf(const struct elf64_hdr *hdr) {
    return hdr->e_ident[0] == ELFMAG0 &&
           hdr->e_ident[1] == ELFMAG1 &&
           hdr->e_ident[2] == ELFMAG2 &&
           hdr->e_ident[3] == ELFMAG3 &&
           hdr->e_ident[4] == ELFCLASS64 &&
           hdr->e_machine == EM_AARCH64;
}

/**
 * 解析ARM64 SUB指令，提取立即数
 * SUB W9, W9, #imm3
 * 指令格式：0x11000000 | (imm3 << 10) | (Rn << 5) | Rd
 */
int32_t parse_arm64_sub_instruction(uint32_t instr) {
    // 检查指令基本格式
    if ((instr & 0x9F000000) != 0x11000000) {
        return -1;
    }
    
    // 提取Rn和Rd寄存器编号
    uint32_t rn = (instr >> 5) & 0x1F;
    uint32_t rd = instr & 0x1F;
    
    // 检查Rn和Rd是否都是W9（寄存器编号9）
    if (rn != 9 || rd != 9) {
        return -1;
    }
    
    // 提取立即数
    uint32_t imm3 = (instr >> 10) & 0x3FF;
    
    return (int32_t)imm3;
}

/**
 * 在.text段中搜索SUB指令
 */
int search_sub_instructions(const uint8_t *text_data, size_t text_size) {
    int main_key = -1;
    
    // 以4字节为单位遍历
    for (size_t i = 0; i + 4 <= text_size; i += 4) {
        // 读取32位指令（小端序）
        uint32_t instr = *(uint32_t *)(text_data + i);
        
        // 解析指令
        int32_t key = parse_arm64_sub_instruction(instr);
        if (key != -1) {
            // 保存第一个找到的密钥作为主密钥
            if (main_key == -1) {
                main_key = key;
            }
        }
    }
    
    return main_key;
}

/**
 * 从ELF文件中提取密钥
 */
int extract_key_from_so(const char *file_path) {
    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        return -1;
    }
    
    // 读取ELF头
    struct elf64_hdr hdr;
    if (fread(&hdr, sizeof(hdr), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }
    
    // 验证ELF文件
    if (!is_valid_elf(&hdr)) {
        fclose(fp);
        return -1;
    }
    
    // 读取节头字符串表
    struct elf64_shdr shdr_strtab;
    if (fseek(fp, hdr.e_shoff + hdr.e_shstrndx * sizeof(struct elf64_shdr), SEEK_SET) != 0) {
        fclose(fp);
        return -1;
    }
    
    if (fread(&shdr_strtab, sizeof(shdr_strtab), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }
    
    // 分配内存读取节头字符串表内容
    char *strtab = (char *)malloc(shdr_strtab.sh_size);
    if (!strtab) {
        fclose(fp);
        return -1;
    }
    
    if (fseek(fp, shdr_strtab.sh_offset, SEEK_SET) != 0) {
        free(strtab);
        fclose(fp);
        return -1;
    }
    
    if (fread(strtab, shdr_strtab.sh_size, 1, fp) != 1) {
        free(strtab);
        fclose(fp);
        return -1;
    }
    
    // 遍历所有节头，查找.text段
    bool found_text = false;
    struct elf64_shdr text_shdr;
    
    for (uint16_t i = 0; i < hdr.e_shnum; i++) {
        if (fseek(fp, hdr.e_shoff + i * sizeof(struct elf64_shdr), SEEK_SET) != 0) {
            continue;
        }
        
        struct elf64_shdr shdr;
        if (fread(&shdr, sizeof(shdr), 1, fp) != 1) {
            continue;
        }
        
        // 获取节名称
        const char *name = strtab + shdr.sh_name;
        
        // 查找.text段
        if (strcmp(name, ".text") == 0) {
            text_shdr = shdr;
            found_text = true;
            break;
        }
    }
    
    free(strtab);
    
    if (!found_text) {
        fclose(fp);
        return -1;
    }
    
    // 读取.text段内容
    size_t text_size = text_shdr.sh_size;
    uint8_t *text_data = (uint8_t *)malloc(text_size);
    if (!text_data) {
        fclose(fp);
        return -1;
    }
    
    if (fseek(fp, text_shdr.sh_offset, SEEK_SET) != 0) {
        free(text_data);
        fclose(fp);
        return -1;
    }
    
    if (fread(text_data, text_size, 1, fp) != 1) {
        free(text_data);
        fclose(fp);
        return -1;
    }
    
    // 搜索SUB指令
    int main_key = search_sub_instructions(text_data, text_size);
    
    free(text_data);
    fclose(fp);
    
    return main_key;
}

/**
 * 检查字符是否为正常可读字符
 * @param c 字符
 * @return 1如果是可读字符，0否则
 */
static int is_normal_char(char c) {
    // 可打印字符范围：空格(32)到波浪号(126)
    return (c >= 32 && c <= 126) || c == '\n' || c == '\t' || c == '\r';
}

/**
 * 分析解密数据，提取最大的连续正常字符序列
 * @param data 解密后的数据
 * @param data_len 数据长度
 * @param result 输出结果
 * @param result_len 结果长度
 */
static void analyze_data(const char *data, size_t data_len, char **result, size_t *result_len) {
    size_t max_start = 0;
    size_t max_len = 0;
    size_t current_start = 0;
    size_t current_len = 0;
    
    // 遍历数据，寻找最大连续正常字符序列
    for (size_t i = 0; i < data_len; i++) {
        if (is_normal_char(data[i])) {
            // 如果是正常字符，增加当前序列长度
            if (current_len == 0) {
                current_start = i;
            }
            current_len++;
            
            // 更新最大序列
            if (current_len > max_len) {
                max_start = current_start;
                max_len = current_len;
            }
        } else {
            // 如果不是正常字符，重置当前序列
            current_len = 0;
        }
    }
    
    // 分配内存存储结果
    *result = (char *)malloc(max_len + 1); // +1 用于结尾的\0
    if (*result != NULL) {
        memcpy(*result, data + max_start, max_len);
        (*result)[max_len] = '\0';
        *result_len = max_len;
    } else {
        *result_len = 0;
    }
}
/**
 * 加密数据函数
 * @param data 要加密的数据
 * @param data_len 数据长度
 * @param key 加密密钥
 */
static void encrypt_data(char *data, size_t data_len, const char *key) {
    size_t key_len = strlen(key);
    for (size_t i = 0; i < data_len; i++) {
        data[i] ^= key[i % key_len];
    }
}

/**
 * 解密数据函数
 * @param data 要解密的数据
 * @param data_len 数据长度
 * @param key 解密密钥
 */
static void decrypt_data(char *data, size_t data_len, const char *key) {
    // XOR加密是对称的，解密和加密使用相同的算法
    encrypt_data(data, data_len, key);
}

/**
 * 解析日期字符串，格式为YYYY-MM-DD
 * @param date_str 日期字符串
 * @param year 输出年份
 * @param month 输出月份
 * @param day 输出日期
 * @return 1成功，0失败
 */
static int parse_date(const char *date_str, int *year, int *month, int *day) {
    return sscanf(date_str, "%d-%d-%d", year, month, day) == 3;
}

/**
 * 比较两个日期，格式为YYYY-MM-DD
 * @param date1 第一个日期字符串
 * @param date2 第二个日期字符串
 * @return -1如果date1<date2，0如果相等，1如果date1>date2
 */
static int compare_dates(const char *date1, const char *date2) {
    int y1, m1, d1;
    int y2, m2, d2;
    
    if (!parse_date(date1, &y1, &m1, &d1) || !parse_date(date2, &y2, &m2, &d2)) {
        return 0;
    }
    
    if (y1 < y2) return -1;
    if (y1 > y2) return 1;
    if (m1 < m2) return -1;
    if (m1 > m2) return 1;
    if (d1 < d2) return -1;
    if (d1 > d2) return 1;
    
    return 0;
}

/**
 * 检查算法注册状态
 * @param L Lua状态机
 * @return 1成功，0失败
 */
static int check_registration(lua_State *L) {
    const char *agreement_path = "/sdcard/XCLUA/Agreement.difierline";
    const char *use_file_path = "/sdcard/Documents/.use";
    
    // 检查Agreement.difierline文件是否存在
    struct stat st;
    if (stat(agreement_path, &st) != 0) {
        luaL_error(L, "未找到注册文件: %s", agreement_path);
        return 0;
    }
    
    // 打开并读取Agreement.difierline文件
    FILE *agreement_file = fopen(agreement_path, "rb+");
    if (agreement_file == NULL) {
        luaL_error(L, "无法打开注册文件: %s", agreement_path);
        return 0;
    }
    
    // 读取文件内容
    char buffer[1024] = {0};
    size_t file_size = fread(buffer, 1, sizeof(buffer) - 1, agreement_file);
    
    // 检查文件大小是否足够（至少包含数据和反转的时间戳）
    if (file_size < 20) {
        fclose(agreement_file);
        luaL_error(L, "注册文件格式错误: 文件太小");
        return 0;
    }
    
    // 检查文件大小，确保至少包含数据和时间戳
    if (file_size < 20) {
        fclose(agreement_file);
        luaL_error(L, "注册文件格式错误: 文件太小");
        return 0;
    }
    
    // 获取文件中的时间戳长度，假设时间戳在10-19位之间
    int timestamp_len = 0;
    for (int i = 19; i >= 10; i--) {
        char test_buf[20] = {0};
        strncpy(test_buf, buffer + file_size - i, i);
        test_buf[i] = '\0';
        
        // 将时间戳反转回原始值
        char test_timestamp[20] = {0};
        strcpy(test_timestamp, test_buf);
        reverse_string(test_timestamp);
        
        // 尝试将反转后的字符串转换为长整数，判断是否为有效时间戳
        char *endptr = NULL;
        long long ts = strtoll(test_timestamp, &endptr, 10);
        if (*endptr == '\0' && ts > 0) {
            timestamp_len = i;
            break;
        }
    }
    
    if (timestamp_len == 0) {
        fclose(agreement_file);
        luaL_error(L, "注册文件格式错误: 无效的时间戳");
        return 0;
    }
    
    // 从文件末尾提取反转的时间戳
    char reversed_timestamp[20] = {0};
    strncpy(reversed_timestamp, buffer + file_size - timestamp_len, timestamp_len);
    reversed_timestamp[timestamp_len] = '\0';
    
    // 将时间戳反转回原始值
    char timestamp_str[20] = {0};
    strcpy(timestamp_str, reversed_timestamp);
    reverse_string(timestamp_str);
    
    // 解密数据部分（排除末尾的反转时间戳）
    size_t data_len = file_size - timestamp_len;
    decrypt_data(buffer, data_len, timestamp_str);
    buffer[data_len] = '\0';
    
    // 解析截止日期
    char end_date[20] = {0};
    char *end_pos = strstr(buffer, "截止日期: ");
    if (end_pos == NULL) {
        fclose(agreement_file);
        luaL_error(L, "注册文件格式错误: 缺少截止日期");
        return 0;
    }
    sscanf(end_pos + 6, "%10s", end_date);
    
    // 获取当前日期
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char current_date[11] = {0};
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", tm_info);
    
    // 检查当前日期是否在有效期内
    if (compare_dates(current_date, end_date) > 0) {
        fclose(agreement_file);
        luaL_error(L, "注册文件已过期");
        return 0;
    }
    
    // 检查是否已使用标记
    int has_used_tag = (strstr(buffer, "已使用") != NULL);
    int use_file_exists = (stat(use_file_path, &st) == 0);
    
    // 第一次使用，添加已使用标记
    if (!has_used_tag) {
        // 构建新的文件内容，添加已使用标记
        char new_content[1024] = {0};
        sprintf(new_content, "截止日期: %s已使用", end_date);
        
        // 使用时间戳作为密钥加密新内容
        size_t new_size = strlen(new_content);
        encrypt_data(new_content, new_size, timestamp_str);
        
        // 重新写入文件，包括反转的时间戳
        char file_content[1024] = {0};
        memcpy(file_content, new_content, new_size);
        strcat(file_content, reversed_timestamp);
        
        // 重新写入文件
        fseek(agreement_file, 0, SEEK_SET);
        fwrite(file_content, 1, strlen(file_content), agreement_file);
        ftruncate(fileno(agreement_file), strlen(file_content));
        
        // 创建.use文件
        FILE *use_file = fopen(use_file_path, "w");
        if (use_file != NULL) {
            fprintf(use_file, "used");
            fclose(use_file);
        }
        fclose(agreement_file);
        
        // 第一次使用，允许注册
        return 1;
    }
    
    fclose(agreement_file);
    
    // 非第一次使用，检查.use文件
    if (has_used_tag && use_file_exists) {
        // 已使用且存在.use文件，允许注册
        return 1;
    } else if (has_used_tag && !use_file_exists) {
        // 已使用但不存在.use文件，拒绝注册
        luaL_error(L, "注册文件验证失败: 缺少使用标记文件");
        return 0;
    } else if (!has_used_tag && use_file_exists) {
        // 未使用但存在.use文件，拒绝注册
        luaL_error(L, "注册文件验证失败: 使用标记文件异常");
        return 0;
    }
    
    return 0;
}
/**
 * 解密单个文件的函数
 * @param L Lua状态机
 * @param in_path 输入文件路径
 * @param out_path 输出文件路径
 * @param key 解密密钥（-1表示自动获取）
 * @param analyze 是否需要分析数据
 * @return 1成功，0失败
 */
static int decrypt_single(lua_State *L, const char *in_path, const char *out_path, int key, int analyze) {
    // 自动获取密钥
    if (key == -1) {
        key = extract_key_from_so(in_path);
        if (key == -1) {
            luaL_error(L, "无法从文件自动获取密钥: %s", in_path);
            return 0;
        }
    }
    
    // 打开输入文件
    FILE *input_file = fopen(in_path, "rb");
    if (input_file == NULL) {
        luaL_error(L, "无法打开输入文件: %s", in_path);
        return 0;
    }

    // 获取文件大小
    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);        
    fseek(input_file, 0, SEEK_SET);

    // 分配内存存储文件内容
    char *input_data = (char *)malloc(file_size);
    if (input_data == NULL) {
        fclose(input_file);
        luaL_error(L, "内存分配失败");  
        return 0;
    }

    // 读取文件内容
    fread(input_data, 1, file_size, input_file);
    fclose(input_file);

    // 执行解密
    for (long i = 0; i < file_size; i++) {     
        input_data[i] = (char)(((unsigned char)input_data[i] - key) % 256);
    }

    // 分配内存存储最终输出数据
    char *final_data = input_data;
    size_t final_size = file_size;

    // 如果需要数据分析
    if (analyze) {
        char *analyzed_data = NULL;
        size_t analyzed_size = 0;

        analyze_data(input_data, file_size, &analyzed_data, &analyzed_size);

        if (analyzed_data != NULL) {
            // 使用分析后的数据
            final_data = analyzed_data;        
            final_size = analyzed_size;        
        }
    }

    // 打开输出文件
    FILE *output_file = fopen(out_path, "wb");
    if (output_file == NULL) {
        if (analyze && final_data != input_data) {
            free(final_data);
        }
        free(input_data);
        luaL_error(L, "无法打开输出文件: %s", out_path);
        return 0;
    }

    // 写入解密结果
    fwrite(final_data, 1, final_size, output_file);
    fclose(output_file);

    // 释放内存
    if (analyze && final_data != input_data) { 
        free(final_data);
    }
    free(input_data);

    return 1;
}

/**
 * 解密文件或目录函数
 * @param L Lua状态机
 * @return 0（无返回值）
 */
static int l_decrypt_file(lua_State *L) {
    if (!check_registration(L)) {
        return 0;
    }
    // 获取输入参数，第三个参数是可选的
    const char *input_path = luaL_checkstring(L, 1);
    const char *output_path = luaL_checkstring(L, 2);
    // 分析选项默认值为true
    int analyze = lua_toboolean(L, 3);
    if (!lua_isboolean(L, 3)) {
        analyze = 1; // 默认值为true
    }

    // 检查输入路径状态
    struct stat st;
    if (stat(input_path, &st) != 0) {
        return luaL_error(L, "无法访问输入路径: %s", input_path);
    }

    // 如果是目录
    if (S_ISDIR(st.st_mode)) {
        // 创建输出目录
        if (mkdir(output_path, 0755) != 0 && errno != EEXIST) {
            return luaL_error(L, "无法创建输出目录: %s", output_path);
        }

        // 打开输入目录
        DIR *dir = opendir(input_path);
        if (dir == NULL) {
            return luaL_error(L, "无法打开输入目录: %s", input_path);
        }

        struct dirent *entry;
        char in_file_path[1024];
        char out_file_path[1024];

        // 遍历目录中的所有文件
        while ((entry = readdir(dir)) != NULL) {
            // 跳过 . 和 ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // 构建完整的输入文件路径
            snprintf(in_file_path, sizeof(in_file_path), "%s/%s", input_path, entry->d_name);
            // 构建完整的输出文件路径
            snprintf(out_file_path, sizeof(out_file_path), "%s/%s", output_path, entry->d_name);

            // 检查是否为文件
            struct stat file_st;
            if (stat(in_file_path, &file_st) == 0 && S_ISREG(file_st.st_mode)) {
                // 解密单个文件，key=-1表示自动获取
                decrypt_single(L, in_file_path, out_file_path, -1, analyze);
            }
        }

        // 关闭目录
        closedir(dir);
    } 
    // 如果是文件
    else if (S_ISREG(st.st_mode)) {
        // 解密单个文件，key=-1表示自动获取
        decrypt_single(L, input_path, output_path, -1, analyze);
    } 
    else {
        return luaL_error(L, "输入路径既不是文件也不是目录: %s", input_path);
    }

    return 0;
}

/**
 * 反转字符串
 * @param str 要反转的字符串
 * @return 反转后的字符串（使用传入的缓冲区）
 */
static char* reverse_string(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
    return str;
}

/**
 * 生成注册文件函数
 * @param L Lua状态机
 * @return 0（无返回值）
 */
static int l_generate_reg_file(lua_State *L) {
    // 只需要截止日期参数
    const char *end_date = luaL_checkstring(L, 1);
    
    const char *reg_file_path = "/sdcard/XCLUA/Agreement.difierline";
    
    // 获取当前时间戳作为密钥
    time_t now = time(NULL);
    char timestamp_str[20] = {0};
    sprintf(timestamp_str, "%ld", now);
    
    // 构建注册文件内容
    char file_content[256] = {0};
    sprintf(file_content, "截止日期: %s", end_date);
    
    // 使用时间戳作为密钥加密内容
    size_t content_len = strlen(file_content);
    encrypt_data(file_content, content_len, timestamp_str);
    
    // 将时间戳反转后附加到加密数据后面
    char reversed_timestamp[20] = {0};
    strcpy(reversed_timestamp, timestamp_str);
    reverse_string(reversed_timestamp);
    strcat(file_content, reversed_timestamp);
    
    // 创建注册文件
    FILE *reg_file = fopen(reg_file_path, "wb");
    if (reg_file == NULL) {
        return luaL_error(L, "无法创建注册文件: %s", reg_file_path);
    }
    
    // 写入加密后的内容
    fwrite(file_content, 1, strlen(file_content), reg_file);
    fclose(reg_file);
    
    return 0;
}

// 模块方法列表
static const luaL_Reg extratool_lib[] = {
    {"extra", l_decrypt_file},
    {NULL, NULL} // 结束标记
};

// 管理员扩展方法列表
static const luaL_Reg admin_lib[] = {
    {"admin", l_generate_reg_file},
    {NULL, NULL} // 结束标记
};



/**
 * 模块初始化函数
 * @param L Lua状态机
 * @return 1（返回模块表）
 */
int luaopen_extratool(lua_State *L) {
    // 检查注册状态

    
    luaL_newlib(L, extratool_lib);
    
    // 检查是否存在admin文件
    const char *admin_file_path = "/sdcard/XCLUA/admin";
    struct stat st;
    if (stat(admin_file_path, &st) == 0) {
        // 如果存在admin文件，添加管理员方法
        luaL_setfuncs(L, admin_lib, 0);
    }
    
    return 1;
}