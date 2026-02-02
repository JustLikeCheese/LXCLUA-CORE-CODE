/**
 * @file lief_elf_wrapper.cpp
 * @brief LIEF ELF C++ API的C语言wrapper实现
 * @description 为LIEF的C++ API提供C语言接口，用于Lua绑定
 * @author DifierLine
 * @date 2026-01-30
 */

#include "lief_elf_wrapper.hpp"

#include <LIEF/ELF.hpp>
#include <LIEF/ELF/NoteDetails/AndroidIdent.hpp>
#include <LIEF/ELF/NoteDetails/NoteAbi.hpp>
#include <memory>
#include <vector>
#include <string>

#ifdef __ANDROID__
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "LIEF", __VA_ARGS__)
#else
#define LOGD(...)
#endif

/* ========== 错误信息存储 ========== */

static thread_local std::string g_last_error;

const char* lief_get_last_error(void) {
    if (g_last_error.empty()) return nullptr;
    return g_last_error.c_str();
}

void lief_clear_error(void) {
    g_last_error.clear();
}

static void set_error(const std::string& msg) {
    g_last_error = msg;
    LOGD("Error: %s", msg.c_str());
}

/* ========== 内部结构定义 ========== */

/**
 * @brief Binary wrapper结构体
 * @description 封装LIEF::ELF::Binary的智能指针
 */
struct Elf_Binary_Wrapper {
    std::unique_ptr<LIEF::ELF::Binary> binary;
    std::string interpreter_cache;      /* 缓存解释器字符串 */
    std::string name_cache;             /* 缓存名称字符串 */
    std::vector<uint8_t> raw_cache;     /* 缓存原始数据 */
    std::vector<uint8_t> content_cache; /* 缓存内容数据 */
};

/* ========== 辅助宏 ========== */

#define CHECK_WRAPPER(w) if (!(w) || !(w)->binary) return
#define CHECK_WRAPPER_RET(w, ret) if (!(w) || !(w)->binary) return (ret)

/* ========== 二进制文件操作实现 ========== */

Elf_Binary_Wrapper* lief_elf_parse(const char* filepath) {
    if (!filepath) return nullptr;
    
    auto binary = LIEF::ELF::Parser::parse(filepath);
    if (!binary) return nullptr;
    
    auto* wrapper = new (std::nothrow) Elf_Binary_Wrapper();
    if (!wrapper) return nullptr;
    
    wrapper->binary = std::move(binary);
    return wrapper;
}

Elf_Binary_Wrapper* lief_elf_parse_from_memory(const uint8_t* data, size_t size) {
    if (!data || size == 0) return nullptr;
    
    std::vector<uint8_t> buffer(data, data + size);
    auto binary = LIEF::ELF::Parser::parse(buffer);
    if (!binary) return nullptr;
    
    auto* wrapper = new (std::nothrow) Elf_Binary_Wrapper();
    if (!wrapper) return nullptr;
    
    wrapper->binary = std::move(binary);
    return wrapper;
}

void lief_elf_destroy(Elf_Binary_Wrapper* wrapper) {
    delete wrapper;
}

int lief_elf_write(Elf_Binary_Wrapper* wrapper, const char* filepath) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!filepath) return -1;
    
    try {
        wrapper->binary->write(filepath);
        return 0;
    } catch (...) {
        return -1;
    }
}

uint8_t* lief_elf_raw(Elf_Binary_Wrapper* wrapper, size_t* out_size) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    if (!out_size) return nullptr;
    
    try {
        wrapper->raw_cache = wrapper->binary->raw();
        *out_size = wrapper->raw_cache.size();
        
        /* 复制数据，调用者负责释放 */
        uint8_t* result = (uint8_t*)malloc(wrapper->raw_cache.size());
        if (result) {
            memcpy(result, wrapper->raw_cache.data(), wrapper->raw_cache.size());
        }
        return result;
    } catch (...) {
        *out_size = 0;
        return nullptr;
    }
}

/* ========== Header操作实现 ========== */

uint64_t lief_elf_get_entrypoint(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->entrypoint();
}

void lief_elf_set_entrypoint(Elf_Binary_Wrapper* wrapper, uint64_t entrypoint) {
    CHECK_WRAPPER(wrapper);
    wrapper->binary->header().entrypoint(entrypoint);
}

uint32_t lief_elf_get_type(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return static_cast<uint32_t>(wrapper->binary->header().file_type());
}

void lief_elf_set_type(Elf_Binary_Wrapper* wrapper, uint32_t type) {
    CHECK_WRAPPER(wrapper);
    wrapper->binary->header().file_type(static_cast<LIEF::ELF::Header::FILE_TYPE>(type));
}

uint32_t lief_elf_get_machine(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return static_cast<uint32_t>(wrapper->binary->header().machine_type());
}

void lief_elf_set_machine(Elf_Binary_Wrapper* wrapper, uint32_t machine) {
    CHECK_WRAPPER(wrapper);
    wrapper->binary->header().machine_type(static_cast<LIEF::ELF::ARCH>(machine));
}

/* ========== 解释器操作实现 ========== */

const char* lief_elf_get_interpreter(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    wrapper->interpreter_cache = wrapper->binary->interpreter();
    return wrapper->interpreter_cache.c_str();
}

void lief_elf_set_interpreter(Elf_Binary_Wrapper* wrapper, const char* interpreter) {
    CHECK_WRAPPER(wrapper);
    if (interpreter) {
        wrapper->binary->interpreter(interpreter);
    }
}

int lief_elf_has_interpreter(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->has_interpreter() ? 1 : 0;
}

/* ========== Section操作实现 ========== */

size_t lief_elf_sections_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->sections().size();
}

const char* lief_elf_section_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return nullptr;
    
    auto it = sections.begin();
    std::advance(it, index);
    wrapper->name_cache = it->name();
    return wrapper->name_cache.c_str();
}

uint64_t lief_elf_section_virtual_address(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return 0;
    
    auto it = sections.begin();
    std::advance(it, index);
    return it->virtual_address();
}

uint64_t lief_elf_section_size(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return 0;
    
    auto it = sections.begin();
    std::advance(it, index);
    return it->size();
}

uint64_t lief_elf_section_offset(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return 0;
    
    auto it = sections.begin();
    std::advance(it, index);
    return it->offset();
}

uint32_t lief_elf_section_type(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return 0;
    
    auto it = sections.begin();
    std::advance(it, index);
    return static_cast<uint32_t>(it->type());
}

uint64_t lief_elf_section_flags(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return 0;
    
    auto it = sections.begin();
    std::advance(it, index);
    return it->flags();
}

const uint8_t* lief_elf_section_content(Elf_Binary_Wrapper* wrapper, size_t index, size_t* out_size) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    if (!out_size) return nullptr;
    
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) {
        *out_size = 0;
        return nullptr;
    }
    
    auto it = sections.begin();
    std::advance(it, index);
    auto content = it->content();
    wrapper->content_cache.assign(content.begin(), content.end());
    *out_size = wrapper->content_cache.size();
    return wrapper->content_cache.data();
}

int lief_elf_section_set_content(Elf_Binary_Wrapper* wrapper, size_t index, 
                                 const uint8_t* content, size_t size) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!content && size > 0) return -1;
    
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return -1;
    
    try {
        auto it = sections.begin();
        std::advance(it, index);
        std::vector<uint8_t> data(content, content + size);
        it->content(std::move(data));
        return 0;
    } catch (...) {
        return -1;
    }
}

int lief_elf_add_section(Elf_Binary_Wrapper* wrapper, const char* name,
                         uint32_t type, uint64_t flags,
                         const uint8_t* content, size_t size, int loaded) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    try {
        LIEF::ELF::Section section;
        section.name(name);
        section.type(static_cast<LIEF::ELF::Section::TYPE>(type));
        section.flags(flags);
        
        if (content && size > 0) {
            std::vector<uint8_t> data(content, content + size);
            section.content(std::move(data));
        }
        
        wrapper->binary->add(section, loaded != 0);
        return static_cast<int>(wrapper->binary->sections().size() - 1);
    } catch (...) {
        return -1;
    }
}

int lief_elf_remove_section(Elf_Binary_Wrapper* wrapper, const char* name, int clear) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    try {
        wrapper->binary->remove_section(name, clear != 0);
        return 0;
    } catch (...) {
        return -1;
    }
}

/* ========== Segment操作实现 ========== */

size_t lief_elf_segments_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->segments().size();
}

uint32_t lief_elf_segment_type(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return 0;
    
    auto it = segments.begin();
    std::advance(it, index);
    return static_cast<uint32_t>(it->type());
}

uint32_t lief_elf_segment_flags(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return 0;
    
    auto it = segments.begin();
    std::advance(it, index);
    return static_cast<uint32_t>(it->flags());
}

uint64_t lief_elf_segment_virtual_address(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return 0;
    
    auto it = segments.begin();
    std::advance(it, index);
    return it->virtual_address();
}

uint64_t lief_elf_segment_virtual_size(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return 0;
    
    auto it = segments.begin();
    std::advance(it, index);
    return it->virtual_size();
}

uint64_t lief_elf_segment_offset(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return 0;
    
    auto it = segments.begin();
    std::advance(it, index);
    return it->file_offset();
}

uint64_t lief_elf_segment_file_size(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return 0;
    
    auto it = segments.begin();
    std::advance(it, index);
    return it->physical_size();
}

const uint8_t* lief_elf_segment_content(Elf_Binary_Wrapper* wrapper, size_t index, size_t* out_size) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    if (!out_size) return nullptr;
    
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) {
        *out_size = 0;
        return nullptr;
    }
    
    auto it = segments.begin();
    std::advance(it, index);
    auto content = it->content();
    wrapper->content_cache.assign(content.begin(), content.end());
    *out_size = wrapper->content_cache.size();
    return wrapper->content_cache.data();
}

int lief_elf_segment_set_content(Elf_Binary_Wrapper* wrapper, size_t index, 
                                 const uint8_t* content, size_t size) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!content && size > 0) return -1;
    
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return -1;
    
    try {
        auto it = segments.begin();
        std::advance(it, index);
        std::vector<uint8_t> data(content, content + size);
        it->content(std::move(data));
        return 0;
    } catch (...) {
        return -1;
    }
}

int lief_elf_add_segment(Elf_Binary_Wrapper* wrapper, uint32_t type, uint32_t flags,
                         const uint8_t* content, size_t size, uint64_t alignment) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    try {
        LIEF::ELF::Segment segment;
        segment.type(static_cast<LIEF::ELF::Segment::TYPE>(type));
        segment.flags(flags);
        segment.alignment(alignment);
        
        if (content && size > 0) {
            std::vector<uint8_t> data(content, content + size);
            segment.content(std::move(data));
        }
        
        wrapper->binary->add(segment);
        return static_cast<int>(wrapper->binary->segments().size() - 1);
    } catch (...) {
        return -1;
    }
}

/* ========== 符号操作实现 ========== */

size_t lief_elf_dynamic_symbols_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->dynamic_symbols().size();
}

size_t lief_elf_symtab_symbols_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->symtab_symbols().size();
}

const char* lief_elf_dynamic_symbol_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    auto symbols = wrapper->binary->dynamic_symbols();
    if (index >= symbols.size()) return nullptr;
    
    auto it = symbols.begin();
    std::advance(it, index);
    wrapper->name_cache = it->name();
    return wrapper->name_cache.c_str();
}

uint64_t lief_elf_dynamic_symbol_value(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto symbols = wrapper->binary->dynamic_symbols();
    if (index >= symbols.size()) return 0;
    
    auto it = symbols.begin();
    std::advance(it, index);
    return it->value();
}

uint64_t lief_elf_dynamic_symbol_size(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto symbols = wrapper->binary->dynamic_symbols();
    if (index >= symbols.size()) return 0;
    
    auto it = symbols.begin();
    std::advance(it, index);
    return it->size();
}

uint32_t lief_elf_dynamic_symbol_type(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto symbols = wrapper->binary->dynamic_symbols();
    if (index >= symbols.size()) return 0;
    
    auto it = symbols.begin();
    std::advance(it, index);
    return static_cast<uint32_t>(it->type());
}

uint32_t lief_elf_dynamic_symbol_binding(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto symbols = wrapper->binary->dynamic_symbols();
    if (index >= symbols.size()) return 0;
    
    auto it = symbols.begin();
    std::advance(it, index);
    return static_cast<uint32_t>(it->binding());
}

int lief_elf_add_dynamic_symbol(Elf_Binary_Wrapper* wrapper, const char* name,
                                uint64_t value, uint64_t size,
                                uint32_t type, uint32_t binding) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    try {
        LIEF::ELF::Symbol symbol;
        symbol.name(name);
        symbol.value(value);
        symbol.size(size);
        symbol.type(static_cast<LIEF::ELF::Symbol::TYPE>(type));
        symbol.binding(static_cast<LIEF::ELF::Symbol::BINDING>(binding));
        
        wrapper->binary->add_dynamic_symbol(symbol);
        return static_cast<int>(wrapper->binary->dynamic_symbols().size() - 1);
    } catch (...) {
        return -1;
    }
}

int lief_elf_remove_dynamic_symbol(Elf_Binary_Wrapper* wrapper, const char* name) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    try {
        wrapper->binary->remove_dynamic_symbol(name);
        return 0;
    } catch (...) {
        return -1;
    }
}

int lief_elf_export_symbol(Elf_Binary_Wrapper* wrapper, const char* name, uint64_t value) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    try {
        wrapper->binary->export_symbol(name, value);
        return 0;
    } catch (...) {
        return -1;
    }
}

/* ========== 动态条目操作实现 ========== */

size_t lief_elf_dynamic_entries_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->dynamic_entries().size();
}

uint64_t lief_elf_dynamic_entry_tag(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto entries = wrapper->binary->dynamic_entries();
    if (index >= entries.size()) return 0;
    
    auto it = entries.begin();
    std::advance(it, index);
    return static_cast<uint64_t>(it->tag());
}

uint64_t lief_elf_dynamic_entry_value(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto entries = wrapper->binary->dynamic_entries();
    if (index >= entries.size()) return 0;
    
    auto it = entries.begin();
    std::advance(it, index);
    return it->value();
}

int lief_elf_remove_dynamic_entry(Elf_Binary_Wrapper* wrapper, uint64_t tag) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    try {
        wrapper->binary->remove(static_cast<LIEF::ELF::DynamicEntry::TAG>(tag));
        return 0;
    } catch (...) {
        return -1;
    }
}

/* ========== 库依赖操作实现 ========== */

int lief_elf_add_library(Elf_Binary_Wrapper* wrapper, const char* library_name) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!library_name) return -1;
    
    try {
        wrapper->binary->add_library(library_name);
        return 0;
    } catch (...) {
        return -1;
    }
}

int lief_elf_remove_library(Elf_Binary_Wrapper* wrapper, const char* library_name) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!library_name) return -1;
    
    try {
        wrapper->binary->remove_library(library_name);
        return 0;
    } catch (...) {
        return -1;
    }
}

int lief_elf_has_library(Elf_Binary_Wrapper* wrapper, const char* library_name) {
    CHECK_WRAPPER_RET(wrapper, 0);
    if (!library_name) return 0;
    return wrapper->binary->has_library(library_name) ? 1 : 0;
}

/* ========== Patch操作实现 ========== */

int lief_elf_patch_address(Elf_Binary_Wrapper* wrapper, uint64_t address,
                           const uint8_t* patch, size_t size) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!patch || size == 0) return -1;
    
    try {
        std::vector<uint8_t> data(patch, patch + size);
        wrapper->binary->patch_address(address, data);
        return 0;
    } catch (...) {
        return -1;
    }
}

int lief_elf_patch_address_value(Elf_Binary_Wrapper* wrapper, uint64_t address,
                                 uint64_t value, size_t size) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (size == 0 || size > 8) return -1;
    
    try {
        wrapper->binary->patch_address(address, value, size);
        return 0;
    } catch (...) {
        return -1;
    }
}

int lief_elf_patch_pltgot(Elf_Binary_Wrapper* wrapper, const char* symbol_name, uint64_t address) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!symbol_name) return -1;
    
    try {
        wrapper->binary->patch_pltgot(symbol_name, address);
        return 0;
    } catch (...) {
        return -1;
    }
}

/* ========== 其他功能实现 ========== */

void lief_elf_strip(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER(wrapper);
    wrapper->binary->strip();
}

int lief_elf_is_pie(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->is_pie() ? 1 : 0;
}

int lief_elf_has_nx(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->has_nx() ? 1 : 0;
}

uint64_t lief_elf_imagebase(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->imagebase();
}

int lief_elf_va_to_offset(Elf_Binary_Wrapper* wrapper, uint64_t virtual_address, uint64_t* out_offset) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!out_offset) return -1;
    
    auto result = wrapper->binary->virtual_address_to_offset(virtual_address);
    if (result) {
        *out_offset = *result;
        return 0;
    }
    return -1;
}

int lief_elf_offset_to_va(Elf_Binary_Wrapper* wrapper, uint64_t offset, uint64_t* out_va) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!out_va) return -1;
    
    auto result = wrapper->binary->offset_to_virtual_address(offset);
    if (result) {
        *out_va = *result;
        return 0;
    }
    return -1;
}

int lief_is_elf(const char* filepath) {
    if (!filepath) return 0;
    return LIEF::ELF::is_elf(filepath) ? 1 : 0;
}

/* ========== 重定位操作实现 ========== */

size_t lief_elf_relocations_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->relocations().size();
}

uint64_t lief_elf_relocation_address(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto relocs = wrapper->binary->relocations();
    if (index >= relocs.size()) return 0;
    
    auto it = relocs.begin();
    std::advance(it, index);
    return it->address();
}

uint32_t lief_elf_relocation_type(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto relocs = wrapper->binary->relocations();
    if (index >= relocs.size()) return 0;
    
    auto it = relocs.begin();
    std::advance(it, index);
    return static_cast<uint32_t>(it->type());
}

int64_t lief_elf_relocation_addend(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto relocs = wrapper->binary->relocations();
    if (index >= relocs.size()) return 0;
    
    auto it = relocs.begin();
    std::advance(it, index);
    return it->addend();
}

/* ========== 扩展Section操作实现 ========== */

uint64_t lief_elf_section_alignment(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return 0;
    
    auto it = sections.begin();
    std::advance(it, index);
    return it->alignment();
}

int lief_elf_section_set_alignment(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t alignment) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return -1;
    
    auto it = sections.begin();
    std::advance(it, index);
    it->alignment(alignment);
    return 0;
}

uint64_t lief_elf_section_entry_size(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return 0;
    
    auto it = sections.begin();
    std::advance(it, index);
    return it->entry_size();
}

int lief_elf_section_set_entry_size(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t entry_size) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return -1;
    
    auto it = sections.begin();
    std::advance(it, index);
    it->entry_size(entry_size);
    return 0;
}

uint32_t lief_elf_section_info(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return 0;
    
    auto it = sections.begin();
    std::advance(it, index);
    return it->information();
}

int lief_elf_section_set_info(Elf_Binary_Wrapper* wrapper, size_t index, uint32_t info) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return -1;
    
    auto it = sections.begin();
    std::advance(it, index);
    it->information(info);
    return 0;
}

uint32_t lief_elf_section_link(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return 0;
    
    auto it = sections.begin();
    std::advance(it, index);
    return it->link();
}

int lief_elf_section_set_link(Elf_Binary_Wrapper* wrapper, size_t index, uint32_t link) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return -1;
    
    auto it = sections.begin();
    std::advance(it, index);
    it->link(link);
    return 0;
}

int lief_elf_section_set_type(Elf_Binary_Wrapper* wrapper, size_t index, uint32_t type) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return -1;
    
    auto it = sections.begin();
    std::advance(it, index);
    it->type(static_cast<LIEF::ELF::Section::TYPE>(type));
    return 0;
}

int lief_elf_section_set_flags(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t flags) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return -1;
    
    auto it = sections.begin();
    std::advance(it, index);
    it->flags(flags);
    return 0;
}

int lief_elf_section_set_virtual_address(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t va) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto sections = wrapper->binary->sections();
    if (index >= sections.size()) return -1;
    
    auto it = sections.begin();
    std::advance(it, index);
    it->virtual_address(va);
    return 0;
}

int lief_elf_get_section_index(Elf_Binary_Wrapper* wrapper, const char* name) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    auto* section = wrapper->binary->get_section(name);
    if (!section) return -1;
    
    auto result = wrapper->binary->get_section_idx(name);
    if (result) {
        return static_cast<int>(*result);
    }
    return -1;
}

int lief_elf_has_section(Elf_Binary_Wrapper* wrapper, const char* name) {
    CHECK_WRAPPER_RET(wrapper, 0);
    if (!name) return 0;
    return wrapper->binary->has_section(name) ? 1 : 0;
}

/* ========== 扩展Segment操作实现 ========== */

uint64_t lief_elf_segment_physical_address(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return 0;
    
    auto it = segments.begin();
    std::advance(it, index);
    return it->physical_address();
}

int lief_elf_segment_set_physical_address(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t paddr) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return -1;
    
    auto it = segments.begin();
    std::advance(it, index);
    it->physical_address(paddr);
    return 0;
}

uint64_t lief_elf_segment_alignment(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return 0;
    
    auto it = segments.begin();
    std::advance(it, index);
    return it->alignment();
}

int lief_elf_segment_set_alignment(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t alignment) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return -1;
    
    auto it = segments.begin();
    std::advance(it, index);
    it->alignment(alignment);
    return 0;
}

int lief_elf_segment_set_type(Elf_Binary_Wrapper* wrapper, size_t index, uint32_t type) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return -1;
    
    auto it = segments.begin();
    std::advance(it, index);
    it->type(static_cast<LIEF::ELF::Segment::TYPE>(type));
    return 0;
}

int lief_elf_segment_set_flags(Elf_Binary_Wrapper* wrapper, size_t index, uint32_t flags) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return -1;
    
    auto it = segments.begin();
    std::advance(it, index);
    it->flags(flags);
    return 0;
}

int lief_elf_segment_set_virtual_address(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t va) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return -1;
    
    auto it = segments.begin();
    std::advance(it, index);
    it->virtual_address(va);
    return 0;
}

int lief_elf_segment_set_virtual_size(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t size) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return -1;
    
    auto it = segments.begin();
    std::advance(it, index);
    it->virtual_size(size);
    return 0;
}

int lief_elf_segment_set_file_offset(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t offset) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return -1;
    
    auto it = segments.begin();
    std::advance(it, index);
    it->file_offset(offset);
    return 0;
}

int lief_elf_segment_set_physical_size(Elf_Binary_Wrapper* wrapper, size_t index, uint64_t size) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return -1;
    
    auto it = segments.begin();
    std::advance(it, index);
    it->physical_size(size);
    return 0;
}

int lief_elf_get_segment_index(Elf_Binary_Wrapper* wrapper, uint32_t type) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    auto seg_type = static_cast<LIEF::ELF::Segment::TYPE>(type);
    auto segments = wrapper->binary->segments();
    
    size_t idx = 0;
    for (auto it = segments.begin(); it != segments.end(); ++it, ++idx) {
        if (it->type() == seg_type) {
            return static_cast<int>(idx);
        }
    }
    return -1;
}

int lief_elf_has_segment(Elf_Binary_Wrapper* wrapper, uint32_t type) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->has(static_cast<LIEF::ELF::Segment::TYPE>(type)) ? 1 : 0;
}

int lief_elf_remove_segment(Elf_Binary_Wrapper* wrapper, size_t index, int clear) {
    CHECK_WRAPPER_RET(wrapper, -1);
    auto segments = wrapper->binary->segments();
    if (index >= segments.size()) return -1;
    
    try {
        auto it = segments.begin();
        std::advance(it, index);
        wrapper->binary->remove(*it, clear != 0);
        return 0;
    } catch (...) {
        return -1;
    }
}

/* ========== symtab符号操作实现 ========== */

const char* lief_elf_symtab_symbol_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    auto symbols = wrapper->binary->symtab_symbols();
    if (index >= symbols.size()) return nullptr;
    
    auto it = symbols.begin();
    std::advance(it, index);
    wrapper->name_cache = it->name();
    return wrapper->name_cache.c_str();
}

uint64_t lief_elf_symtab_symbol_value(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto symbols = wrapper->binary->symtab_symbols();
    if (index >= symbols.size()) return 0;
    
    auto it = symbols.begin();
    std::advance(it, index);
    return it->value();
}

uint64_t lief_elf_symtab_symbol_size(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto symbols = wrapper->binary->symtab_symbols();
    if (index >= symbols.size()) return 0;
    
    auto it = symbols.begin();
    std::advance(it, index);
    return it->size();
}

uint32_t lief_elf_symtab_symbol_type(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto symbols = wrapper->binary->symtab_symbols();
    if (index >= symbols.size()) return 0;
    
    auto it = symbols.begin();
    std::advance(it, index);
    return static_cast<uint32_t>(it->type());
}

uint32_t lief_elf_symtab_symbol_binding(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto symbols = wrapper->binary->symtab_symbols();
    if (index >= symbols.size()) return 0;
    
    auto it = symbols.begin();
    std::advance(it, index);
    return static_cast<uint32_t>(it->binding());
}

int lief_elf_add_symtab_symbol(Elf_Binary_Wrapper* wrapper, const char* name,
                               uint64_t value, uint64_t size,
                               uint32_t type, uint32_t binding) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    try {
        LIEF::ELF::Symbol symbol;
        symbol.name(name);
        symbol.value(value);
        symbol.size(size);
        symbol.type(static_cast<LIEF::ELF::Symbol::TYPE>(type));
        symbol.binding(static_cast<LIEF::ELF::Symbol::BINDING>(binding));
        
        wrapper->binary->add_symtab_symbol(symbol);
        return static_cast<int>(wrapper->binary->symtab_symbols().size() - 1);
    } catch (...) {
        return -1;
    }
}

int lief_elf_remove_symtab_symbol(Elf_Binary_Wrapper* wrapper, const char* name) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    try {
        wrapper->binary->remove_symtab_symbol(name);
        return 0;
    } catch (...) {
        return -1;
    }
}

/* ========== 内存内容操作实现 ========== */

const uint8_t* lief_elf_get_content_from_va(Elf_Binary_Wrapper* wrapper, uint64_t va, 
                                            uint64_t size, size_t* out_size) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    if (!out_size) return nullptr;
    
    auto content = wrapper->binary->get_content_from_virtual_address(va, size);
    wrapper->content_cache.assign(content.begin(), content.end());
    *out_size = wrapper->content_cache.size();
    return wrapper->content_cache.data();
}

int lief_elf_section_from_offset(Elf_Binary_Wrapper* wrapper, uint64_t offset) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    auto* section = wrapper->binary->section_from_offset(offset);
    if (!section) return -1;
    
    auto result = wrapper->binary->get_section_idx(*section);
    if (result) {
        return static_cast<int>(*result);
    }
    return -1;
}

int lief_elf_section_from_va(Elf_Binary_Wrapper* wrapper, uint64_t va) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    auto* section = wrapper->binary->section_from_virtual_address(va);
    if (!section) return -1;
    
    auto result = wrapper->binary->get_section_idx(*section);
    if (result) {
        return static_cast<int>(*result);
    }
    return -1;
}

int lief_elf_segment_from_offset(Elf_Binary_Wrapper* wrapper, uint64_t offset) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    auto* segment = wrapper->binary->segment_from_offset(offset);
    if (!segment) return -1;
    
    auto segments = wrapper->binary->segments();
    size_t idx = 0;
    for (auto it = segments.begin(); it != segments.end(); ++it, ++idx) {
        if (&(*it) == segment) {
            return static_cast<int>(idx);
        }
    }
    return -1;
}

int lief_elf_segment_from_va(Elf_Binary_Wrapper* wrapper, uint64_t va) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    auto* segment = wrapper->binary->segment_from_virtual_address(va);
    if (!segment) return -1;
    
    auto segments = wrapper->binary->segments();
    size_t idx = 0;
    for (auto it = segments.begin(); it != segments.end(); ++it, ++idx) {
        if (&(*it) == segment) {
            return static_cast<int>(idx);
        }
    }
    return -1;
}

/* ========== 二进制信息实现 ========== */

uint64_t lief_elf_virtual_size(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->virtual_size();
}

uint64_t lief_elf_eof_offset(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->eof_offset();
}

int lief_elf_is_targeting_android(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->is_targeting_android() ? 1 : 0;
}

int lief_elf_has_overlay(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->has_overlay() ? 1 : 0;
}

const uint8_t* lief_elf_get_overlay(Elf_Binary_Wrapper* wrapper, size_t* out_size) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    if (!out_size) return nullptr;
    
    auto overlay = wrapper->binary->overlay();
    wrapper->content_cache.assign(overlay.begin(), overlay.end());
    *out_size = wrapper->content_cache.size();
    return wrapper->content_cache.data();
}

int lief_elf_set_overlay(Elf_Binary_Wrapper* wrapper, const uint8_t* data, size_t size) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    try {
        std::vector<uint8_t> overlay_data;
        if (data && size > 0) {
            overlay_data.assign(data, data + size);
        }
        wrapper->binary->overlay(std::move(overlay_data));
        return 0;
    } catch (...) {
        return -1;
    }
}

/* ========== 添加动态重定位实现 ========== */

int lief_elf_add_dynamic_relocation(Elf_Binary_Wrapper* wrapper, uint64_t address,
                                    uint32_t type, int64_t addend,
                                    const char* symbol_name) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    try {
        LIEF::ELF::Relocation reloc;
        reloc.address(address);
        reloc.type(static_cast<LIEF::ELF::Relocation::TYPE>(type));
        reloc.addend(addend);
        
        if (symbol_name) {
            auto* sym = wrapper->binary->get_dynamic_symbol(symbol_name);
            if (sym) {
                reloc.symbol(sym);
            }
        }
        
        wrapper->binary->add_dynamic_relocation(reloc);
        return 0;
    } catch (...) {
        return -1;
    }
}

int lief_elf_add_pltgot_relocation(Elf_Binary_Wrapper* wrapper, uint64_t address,
                                   uint32_t type, const char* symbol_name) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!symbol_name) return -1;
    
    try {
        LIEF::ELF::Relocation reloc;
        reloc.address(address);
        reloc.type(static_cast<LIEF::ELF::Relocation::TYPE>(type));
        
        auto* sym = wrapper->binary->get_dynamic_symbol(symbol_name);
        if (sym) {
            reloc.symbol(sym);
        }
        
        wrapper->binary->add_pltgot_relocation(reloc);
        return 0;
    } catch (...) {
        return -1;
    }
}

/* ========== 扩展动态条目操作实现 ========== */

int lief_elf_get_dynamic_entry_by_tag(Elf_Binary_Wrapper* wrapper, uint64_t tag, uint64_t* out_value) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!out_value) return -1;
    
    auto* entry = wrapper->binary->get(static_cast<LIEF::ELF::DynamicEntry::TAG>(tag));
    if (!entry) return -1;
    
    *out_value = entry->value();
    return 0;
}

int lief_elf_has_dynamic_entry(Elf_Binary_Wrapper* wrapper, uint64_t tag) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->has(static_cast<LIEF::ELF::DynamicEntry::TAG>(tag)) ? 1 : 0;
}

/* ========== 反汇编操作实现 ========== */

Disasm_Instruction* lief_elf_disassemble(Elf_Binary_Wrapper* wrapper, uint64_t address, 
                                         size_t size, size_t* out_count) {
    lief_clear_error();
    CHECK_WRAPPER_RET(wrapper, nullptr);
    if (!out_count) {
        set_error("out_count parameter is null");
        return nullptr;
    }
    
    LOGD("disassemble: address=0x%llx, size=%zu", (unsigned long long)address, size);
    
    /* 获取可执行内容用于验证地址 */
    auto content = wrapper->binary->get_content_from_virtual_address(address, size);
    if (content.empty()) {
        /* 地址无效，尝试提供更多信息 */
        char buf[512];
        uint64_t imagebase = wrapper->binary->imagebase();
        
        /* 查找.text section的地址范围 */
        auto* text_sec = wrapper->binary->get_section(".text");
        if (text_sec) {
            snprintf(buf, sizeof(buf), 
                "Cannot read content at address 0x%llx. "
                "Imagebase=0x%llx, .text section: VA=0x%llx, Size=0x%llx. "
                "Try using an address within the .text section range.",
                (unsigned long long)address,
                (unsigned long long)imagebase,
                (unsigned long long)text_sec->virtual_address(),
                (unsigned long long)text_sec->size());
        } else {
            snprintf(buf, sizeof(buf), 
                "Cannot read content at address 0x%llx. Imagebase=0x%llx. "
                "The address may not be mapped in the binary.",
                (unsigned long long)address,
                (unsigned long long)imagebase);
        }
        set_error(buf);
        *out_count = 0;
        return nullptr;
    }
    
    LOGD("disassemble: got %zu bytes of content", content.size());
    
    try {
        auto insts = wrapper->binary->disassemble(address, size);
        
        /* 先计数 */
        size_t count = 0;
        for (auto it = insts.begin(); it != insts.end(); ++it) {
            count++;
        }
        
        LOGD("disassemble: found %zu instructions", count);
        
        if (count == 0) {
            char buf[256];
            snprintf(buf, sizeof(buf), 
                "No instructions found at address 0x%llx (size=%zu). "
                "Check if address is valid and within executable section.",
                (unsigned long long)address, size);
            set_error(buf);
            *out_count = 0;
            return nullptr;
        }
        
        /* 重新获取迭代器并分配内存 */
        auto insts2 = wrapper->binary->disassemble(address, size);
        Disasm_Instruction* result = (Disasm_Instruction*)calloc(count, sizeof(Disasm_Instruction));
        if (!result) {
            set_error("Failed to allocate memory for instructions");
            *out_count = 0;
            return nullptr;
        }
        
        size_t idx = 0;
        for (auto inst : insts2) {
            if (!inst || idx >= count) break;
            
            result[idx].address = inst->address();
            
            std::string mnem = inst->mnemonic();
            strncpy(result[idx].mnemonic, mnem.c_str(), sizeof(result[idx].mnemonic) - 1);
            
            std::string full = inst->to_string(false);
            strncpy(result[idx].full_str, full.c_str(), sizeof(result[idx].full_str) - 1);
            
            /* 分离操作数 */
            size_t space_pos = full.find(' ');
            if (space_pos != std::string::npos && space_pos + 1 < full.length()) {
                std::string ops = full.substr(space_pos + 1);
                strncpy(result[idx].operands, ops.c_str(), sizeof(result[idx].operands) - 1);
            }
            
            /* 原始字节 */
            auto raw = inst->raw();
            result[idx].raw_size = std::min(raw.size(), sizeof(result[idx].raw));
            memcpy(result[idx].raw, raw.data(), result[idx].raw_size);
            
            result[idx].is_call = inst->is_call() ? 1 : 0;
            result[idx].is_branch = inst->is_branch() ? 1 : 0;
            result[idx].is_return = inst->is_return() ? 1 : 0;
            
            idx++;
        }
        
        *out_count = idx;
        return result;
    } catch (const std::exception& e) {
        set_error(std::string("Disassemble exception: ") + e.what());
        *out_count = 0;
        return nullptr;
    } catch (...) {
        set_error("Disassemble unknown exception");
        *out_count = 0;
        return nullptr;
    }
}

Disasm_Instruction* lief_elf_disassemble_buffer(Elf_Binary_Wrapper* wrapper, 
                                                const uint8_t* buffer, size_t size,
                                                uint64_t address, size_t* out_count) {
    lief_clear_error();
    CHECK_WRAPPER_RET(wrapper, nullptr);
    if (!out_count || !buffer || size == 0) {
        set_error("Invalid parameters for disassemble_buffer");
        return nullptr;
    }
    
    try {
        auto insts = wrapper->binary->disassemble(buffer, size, address);
        
        /* 先计数 */
        size_t count = 0;
        for (auto it = insts.begin(); it != insts.end(); ++it) {
            count++;
        }
        
        if (count == 0) {
            set_error("No instructions found in buffer");
            *out_count = 0;
            return nullptr;
        }
        
        /* 重新获取迭代器并分配内存 */
        auto insts2 = wrapper->binary->disassemble(buffer, size, address);
        Disasm_Instruction* result = (Disasm_Instruction*)calloc(count, sizeof(Disasm_Instruction));
        if (!result) {
            *out_count = 0;
            return nullptr;
        }
        
        size_t idx = 0;
        for (auto inst : insts2) {
            if (!inst || idx >= count) break;
            
            result[idx].address = inst->address();
            
            std::string mnem = inst->mnemonic();
            strncpy(result[idx].mnemonic, mnem.c_str(), sizeof(result[idx].mnemonic) - 1);
            
            std::string full = inst->to_string(false);
            strncpy(result[idx].full_str, full.c_str(), sizeof(result[idx].full_str) - 1);
            
            size_t space_pos = full.find(' ');
            if (space_pos != std::string::npos && space_pos + 1 < full.length()) {
                std::string ops = full.substr(space_pos + 1);
                strncpy(result[idx].operands, ops.c_str(), sizeof(result[idx].operands) - 1);
            }
            
            auto raw = inst->raw();
            result[idx].raw_size = std::min(raw.size(), sizeof(result[idx].raw));
            memcpy(result[idx].raw, raw.data(), result[idx].raw_size);
            
            result[idx].is_call = inst->is_call() ? 1 : 0;
            result[idx].is_branch = inst->is_branch() ? 1 : 0;
            result[idx].is_return = inst->is_return() ? 1 : 0;
            
            idx++;
        }
        
        *out_count = idx;
        return result;
    } catch (...) {
        *out_count = 0;
        return nullptr;
    }
}

Disasm_Instruction* lief_elf_disassemble_symbol(Elf_Binary_Wrapper* wrapper, 
                                                const char* symbol_name, size_t* out_count) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    if (!out_count || !symbol_name) return nullptr;
    
    try {
        auto insts = wrapper->binary->disassemble(symbol_name);
        
        size_t count = 0;
        for (auto it = insts.begin(); it != insts.end(); ++it) {
            count++;
        }
        
        if (count == 0) {
            *out_count = 0;
            return nullptr;
        }
        
        auto insts2 = wrapper->binary->disassemble(symbol_name);
        Disasm_Instruction* result = (Disasm_Instruction*)calloc(count, sizeof(Disasm_Instruction));
        if (!result) {
            *out_count = 0;
            return nullptr;
        }
        
        size_t idx = 0;
        for (auto inst : insts2) {
            if (!inst || idx >= count) break;
            
            result[idx].address = inst->address();
            
            std::string mnem = inst->mnemonic();
            strncpy(result[idx].mnemonic, mnem.c_str(), sizeof(result[idx].mnemonic) - 1);
            
            std::string full = inst->to_string(false);
            strncpy(result[idx].full_str, full.c_str(), sizeof(result[idx].full_str) - 1);
            
            size_t space_pos = full.find(' ');
            if (space_pos != std::string::npos && space_pos + 1 < full.length()) {
                std::string ops = full.substr(space_pos + 1);
                strncpy(result[idx].operands, ops.c_str(), sizeof(result[idx].operands) - 1);
            }
            
            auto raw = inst->raw();
            result[idx].raw_size = std::min(raw.size(), sizeof(result[idx].raw));
            memcpy(result[idx].raw, raw.data(), result[idx].raw_size);
            
            result[idx].is_call = inst->is_call() ? 1 : 0;
            result[idx].is_branch = inst->is_branch() ? 1 : 0;
            result[idx].is_return = inst->is_return() ? 1 : 0;
            
            idx++;
        }
        
        *out_count = idx;
        return result;
    } catch (const std::exception& e) {
        set_error(std::string("Disassemble symbol exception: ") + e.what());
        *out_count = 0;
        return nullptr;
    } catch (...) {
        set_error("Disassemble symbol unknown exception");
        *out_count = 0;
        return nullptr;
    }
}

void lief_elf_free_disasm(Disasm_Instruction* instructions) {
    free(instructions);
}

uint8_t* lief_elf_assemble(Elf_Binary_Wrapper* wrapper, uint64_t address, 
                           const char* assembly, size_t* out_size) {
    lief_clear_error();
    CHECK_WRAPPER_RET(wrapper, nullptr);
    if (!assembly || !out_size) {
        set_error("Invalid parameters for assemble");
        return nullptr;
    }
    
    /* 获取架构信息 */
    uint32_t machine = static_cast<uint32_t>(wrapper->binary->header().machine_type());
    const char* arch_name = "unknown";
    switch (machine) {
        case 0x3: arch_name = "x86"; break;
        case 0x3E: arch_name = "x86_64"; break;
        case 0x28: arch_name = "ARM"; break;
        case 0xB7: arch_name = "AArch64"; break;
        case 0xF3: arch_name = "RISC-V"; break;
    }
    
    LOGD("assemble: address=0x%llx, arch=%s(%u), asm='%s'", 
         (unsigned long long)address, arch_name, machine, assembly);
    
    try {
        auto bytes = wrapper->binary->assemble(address, assembly);
        LOGD("assemble: got %zu bytes", bytes.size());
        if (bytes.empty()) {
            char buf[256];
            snprintf(buf, sizeof(buf), 
                "Assemble returned empty result for '%s' (arch=%s). "
                "Ensure LIEF was compiled with LLVM support and the syntax is correct for the target architecture.",
                assembly, arch_name);
            set_error(buf);
            *out_size = 0;
            return nullptr;
        }
        
        uint8_t* result = (uint8_t*)malloc(bytes.size());
        if (!result) {
            set_error("Failed to allocate memory for assembled bytes");
            *out_size = 0;
            return nullptr;
        }
        
        memcpy(result, bytes.data(), bytes.size());
        *out_size = bytes.size();
        return result;
    } catch (const std::exception& e) {
        char buf[512];
        snprintf(buf, sizeof(buf), "Assemble exception (arch=%s): %s", arch_name, e.what());
        set_error(buf);
        LOGD("assemble exception: %s", e.what());
        *out_size = 0;
        return nullptr;
    } catch (...) {
        LOGD("assemble unknown exception");
        set_error("Assemble unknown exception");
        *out_size = 0;
        return nullptr;
    }
}

int lief_elf_assemble_patch(Elf_Binary_Wrapper* wrapper, uint64_t address, 
                            const char* assembly) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!assembly) return -1;
    
    try {
        wrapper->binary->assemble(address, assembly);
        return 0;
    } catch (...) {
        return -1;
    }
}

/* ========== Header扩展操作实现 ========== */

uint32_t lief_elf_get_class(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return static_cast<uint32_t>(wrapper->binary->header().identity_class());
}

uint32_t lief_elf_get_endianness(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return static_cast<uint32_t>(wrapper->binary->header().identity_data());
}

uint32_t lief_elf_get_version(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return static_cast<uint32_t>(wrapper->binary->header().identity_version());
}

uint64_t lief_elf_get_program_header_offset(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->header().program_headers_offset();
}

uint64_t lief_elf_get_section_header_offset(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->header().section_headers_offset();
}

uint32_t lief_elf_get_program_header_size(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->header().program_header_size();
}

uint32_t lief_elf_get_section_header_size(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->header().section_header_size();
}

uint32_t lief_elf_get_section_name_index(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->header().section_name_table_idx();
}

uint32_t lief_elf_get_header_flags(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->header().processor_flag();
}

/* ========== 库依赖扩展实现 ========== */

size_t lief_elf_libraries_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->imported_libraries().size();
}

const char* lief_elf_library_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    auto libs = wrapper->binary->imported_libraries();
    if (index >= libs.size()) return nullptr;
    
    wrapper->name_cache = libs[index];
    return wrapper->name_cache.c_str();
}

/* ========== 导出/导入符号实现 ========== */

size_t lief_elf_exported_functions_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->exported_functions().size();
}

const char* lief_elf_exported_function_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    auto funcs = wrapper->binary->exported_functions();
    if (index >= funcs.size()) return nullptr;
    
    wrapper->name_cache = funcs[index].name();
    return wrapper->name_cache.c_str();
}

uint64_t lief_elf_exported_function_address(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto funcs = wrapper->binary->exported_functions();
    if (index >= funcs.size()) return 0;
    
    return funcs[index].address();
}

size_t lief_elf_imported_functions_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->imported_functions().size();
}

const char* lief_elf_imported_function_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    auto funcs = wrapper->binary->imported_functions();
    if (index >= funcs.size()) return nullptr;
    
    wrapper->name_cache = funcs[index].name();
    return wrapper->name_cache.c_str();
}

/* ========== 构造函数实现 ========== */

size_t lief_elf_ctor_functions_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->ctor_functions().size();
}

uint64_t lief_elf_ctor_function_address(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto funcs = wrapper->binary->ctor_functions();
    if (index >= funcs.size()) return 0;
    
    return funcs[index].address();
}

/* ========== Note操作实现 ========== */

size_t lief_elf_notes_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->notes().size();
}

const char* lief_elf_note_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    auto notes = wrapper->binary->notes();
    if (index >= notes.size()) return nullptr;
    
    auto it = notes.begin();
    std::advance(it, index);
    wrapper->name_cache = it->name();
    return wrapper->name_cache.c_str();
}

uint32_t lief_elf_note_type(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto notes = wrapper->binary->notes();
    if (index >= notes.size()) return 0;
    
    auto it = notes.begin();
    std::advance(it, index);
    return static_cast<uint32_t>(it->type());
}

/* ========== 辅助函数实现 ========== */

const char* lief_format_address(uint64_t address, char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 19) return nullptr;
    snprintf(buffer, buffer_size, "0x%016llX", (unsigned long long)address);
    return buffer;
}

uint64_t lief_elf_page_size(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->page_size();
}

const char* lief_elf_get_soname(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    
    /* 通过动态条目遍历获取SONAME */
    auto entries = wrapper->binary->dynamic_entries();
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        if (it->tag() == LIEF::ELF::DynamicEntry::TAG::SONAME) {
            /* 使用LIEF提供的API获取库名 */
            auto& entry = *it;
            /* DynamicEntryLibrary继承自DynamicEntry，但我们不使用dynamic_cast */
            /* 使用 static_cast，因为我们已经确认了tag类型 */
            auto* lib_entry = static_cast<const LIEF::ELF::DynamicEntryLibrary*>(&entry);
            wrapper->name_cache = lib_entry->name();
            return wrapper->name_cache.c_str();
        }
    }
    return nullptr;
}

int lief_elf_set_soname(Elf_Binary_Wrapper* wrapper, const char* soname) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!soname) return -1;
    
    try {
        auto* entry = wrapper->binary->get(LIEF::ELF::DynamicEntry::TAG::SONAME);
        if (entry) {
            auto* soname_entry = static_cast<LIEF::ELF::DynamicEntryLibrary*>(entry);
            soname_entry->name(soname);
            return 0;
        }
        return -1;
    } catch (...) {
        return -1;
    }
}

const char* lief_elf_get_runpath(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    
    /* 通过动态条目遍历获取RUNPATH */
    auto entries = wrapper->binary->dynamic_entries();
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        if (it->tag() == LIEF::ELF::DynamicEntry::TAG::RUNPATH) {
            auto& entry = *it;
            auto* runpath_entry = static_cast<const LIEF::ELF::DynamicEntryRunPath*>(&entry);
            wrapper->name_cache = runpath_entry->runpath();
            return wrapper->name_cache.c_str();
        }
    }
    return nullptr;
}

int lief_elf_set_runpath(Elf_Binary_Wrapper* wrapper, const char* runpath) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!runpath) return -1;
    
    try {
        auto* entry = wrapper->binary->get(LIEF::ELF::DynamicEntry::TAG::RUNPATH);
        if (entry) {
            auto* runpath_entry = static_cast<LIEF::ELF::DynamicEntryRunPath*>(entry);
            runpath_entry->runpath(runpath);
            return 0;
        }
        return -1;
    } catch (...) {
        return -1;
    }
}

size_t lief_elf_symbol_versions_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->symbols_version().size();
}

int lief_elf_has_debug_info(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->has_section(".debug_info") ? 1 : 0;
}

int lief_elf_has_gnu_hash(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->has(LIEF::ELF::DynamicEntry::TAG::GNU_HASH) ? 1 : 0;
}

int lief_elf_has_sysv_hash(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->has(LIEF::ELF::DynamicEntry::TAG::HASH) ? 1 : 0;
}

/* ========== 符号查找操作实现 ========== */

int lief_elf_has_dynamic_symbol(Elf_Binary_Wrapper* wrapper, const char* name) {
    CHECK_WRAPPER_RET(wrapper, 0);
    if (!name) return 0;
    return wrapper->binary->has_dynamic_symbol(name) ? 1 : 0;
}

int lief_elf_has_symtab_symbol(Elf_Binary_Wrapper* wrapper, const char* name) {
    CHECK_WRAPPER_RET(wrapper, 0);
    if (!name) return 0;
    return wrapper->binary->has_symtab_symbol(name) ? 1 : 0;
}

int lief_elf_get_dynamic_symbol(Elf_Binary_Wrapper* wrapper, const char* name,
                                uint64_t* out_value, uint64_t* out_size,
                                uint32_t* out_type, uint32_t* out_binding) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    auto* sym = wrapper->binary->get_dynamic_symbol(name);
    if (!sym) return -1;
    
    if (out_value) *out_value = sym->value();
    if (out_size) *out_size = sym->size();
    if (out_type) *out_type = static_cast<uint32_t>(sym->type());
    if (out_binding) *out_binding = static_cast<uint32_t>(sym->binding());
    
    return 0;
}

int lief_elf_get_symtab_symbol(Elf_Binary_Wrapper* wrapper, const char* name,
                               uint64_t* out_value, uint64_t* out_size,
                               uint32_t* out_type, uint32_t* out_binding) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    auto* sym = wrapper->binary->get_symtab_symbol(name);
    if (!sym) return -1;
    
    if (out_value) *out_value = sym->value();
    if (out_size) *out_size = sym->size();
    if (out_type) *out_type = static_cast<uint32_t>(sym->type());
    if (out_binding) *out_binding = static_cast<uint32_t>(sym->binding());
    
    return 0;
}

/* ========== 重定位查找操作实现 ========== */

int lief_elf_get_relocation_by_address(Elf_Binary_Wrapper* wrapper, uint64_t address,
                                       uint32_t* out_type, int64_t* out_addend) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    auto* reloc = wrapper->binary->get_relocation(address);
    if (!reloc) return -1;
    
    if (out_type) *out_type = static_cast<uint32_t>(reloc->type());
    if (out_addend) *out_addend = reloc->addend();
    
    return 0;
}

int lief_elf_get_relocation_by_symbol(Elf_Binary_Wrapper* wrapper, const char* symbol_name,
                                      uint64_t* out_address, uint32_t* out_type, int64_t* out_addend) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!symbol_name) return -1;
    
    auto* reloc = wrapper->binary->get_relocation(symbol_name);
    if (!reloc) return -1;
    
    if (out_address) *out_address = reloc->address();
    if (out_type) *out_type = static_cast<uint32_t>(reloc->type());
    if (out_addend) *out_addend = reloc->addend();
    
    return 0;
}

const char* lief_elf_relocation_symbol_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    auto relocs = wrapper->binary->relocations();
    if (index >= relocs.size()) return nullptr;
    
    auto it = relocs.begin();
    std::advance(it, index);
    
    auto* sym = it->symbol();
    if (!sym) return nullptr;
    
    wrapper->name_cache = sym->name();
    return wrapper->name_cache.c_str();
}

/* ========== PLT/GOT和动态重定位实现 ========== */

size_t lief_elf_pltgot_relocations_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto relocs = wrapper->binary->pltgot_relocations();
    size_t count = 0;
    for (auto it = relocs.begin(); it != relocs.end(); ++it) {
        count++;
    }
    return count;
}

size_t lief_elf_dynamic_relocations_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto relocs = wrapper->binary->dynamic_relocations();
    size_t count = 0;
    for (auto it = relocs.begin(); it != relocs.end(); ++it) {
        count++;
    }
    return count;
}

/* ========== GNU Hash符号检查实现 ========== */

int lief_elf_gnu_hash_check(Elf_Binary_Wrapper* wrapper, const char* symbol_name) {
    CHECK_WRAPPER_RET(wrapper, 0);
    if (!symbol_name) return 0;
    
    auto* gnu_hash = wrapper->binary->gnu_hash();
    if (!gnu_hash) return 0;
    
    return gnu_hash->check(symbol_name) ? 1 : 0;
}

/* ========== 字符串提取实现 ========== */

Elf_String* lief_elf_strings(Elf_Binary_Wrapper* wrapper, size_t min_size, size_t* out_count) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    if (!out_count) return nullptr;
    
    try {
        auto strings = wrapper->binary->strings(min_size);
        if (strings.empty()) {
            *out_count = 0;
            return nullptr;
        }
        
        Elf_String* result = (Elf_String*)calloc(strings.size(), sizeof(Elf_String));
        if (!result) {
            *out_count = 0;
            return nullptr;
        }
        
        for (size_t i = 0; i < strings.size(); i++) {
            result[i].str = strdup(strings[i].c_str());
            result[i].offset = 0; /* LIEF strings()只返回字符串内容 */
        }
        
        *out_count = strings.size();
        return result;
    } catch (...) {
        *out_count = 0;
        return nullptr;
    }
}

void lief_elf_free_strings(Elf_String* strings, size_t count) {
    if (!strings) return;
    for (size_t i = 0; i < count; i++) {
        free(strings[i].str);
    }
    free(strings);
}

/* ========== 虚拟地址操作实现 ========== */

uint64_t lief_elf_next_virtual_address(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->next_virtual_address();
}

uint64_t lief_elf_last_offset_section(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->last_offset_section();
}

uint64_t lief_elf_last_offset_segment(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->last_offset_segment();
}

/* ========== 扩展符号操作实现 ========== */

int lief_elf_remove_symbol(Elf_Binary_Wrapper* wrapper, const char* name) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    try {
        wrapper->binary->remove_symbol(name);
        return 0;
    } catch (...) {
        return -1;
    }
}

size_t lief_elf_imported_symbols_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto syms = wrapper->binary->imported_symbols();
    size_t count = 0;
    for (auto it = syms.begin(); it != syms.end(); ++it) {
        count++;
    }
    return count;
}

size_t lief_elf_exported_symbols_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto syms = wrapper->binary->exported_symbols();
    size_t count = 0;
    for (auto it = syms.begin(); it != syms.end(); ++it) {
        count++;
    }
    return count;
}

const char* lief_elf_imported_symbol_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    auto syms = wrapper->binary->imported_symbols();
    
    size_t idx = 0;
    for (auto it = syms.begin(); it != syms.end(); ++it, ++idx) {
        if (idx == index) {
            wrapper->name_cache = (*it).name();
            return wrapper->name_cache.c_str();
        }
    }
    return nullptr;
}

const char* lief_elf_exported_symbol_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    auto syms = wrapper->binary->exported_symbols();
    
    size_t idx = 0;
    for (auto it = syms.begin(); it != syms.end(); ++it, ++idx) {
        if (idx == index) {
            wrapper->name_cache = (*it).name();
            return wrapper->name_cache.c_str();
        }
    }
    return nullptr;
}

uint64_t lief_elf_exported_symbol_value(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    auto syms = wrapper->binary->exported_symbols();
    
    size_t idx = 0;
    for (auto it = syms.begin(); it != syms.end(); ++it, ++idx) {
        if (idx == index) {
            return (*it).value();
        }
    }
    return 0;
}

/* ========== 安全检查实现 ========== */

int lief_elf_relro_type(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    /* 检查是否有GNU_RELRO段 */
    bool has_relro = wrapper->binary->has(LIEF::ELF::Segment::TYPE::GNU_RELRO);
    if (!has_relro) return 0;
    
    /* 检查是否有BIND_NOW标志 */
    bool has_bind_now = wrapper->binary->has(LIEF::ELF::DynamicEntry::TAG::BIND_NOW);
    
    /* 检查FLAGS中的NOW标志 */
    auto* flags = wrapper->binary->get(LIEF::ELF::DynamicEntry::TAG::FLAGS);
    bool has_flags_now = false;
    if (flags) {
        uint64_t val = flags->value();
        has_flags_now = (val & 0x8) != 0; /* DF_BIND_NOW = 0x8 */
    }
    
    /* 检查FLAGS_1中的NOW标志 */
    auto* flags1 = wrapper->binary->get(LIEF::ELF::DynamicEntry::TAG::FLAGS_1);
    bool has_flags1_now = false;
    if (flags1) {
        uint64_t val = flags1->value();
        has_flags1_now = (val & 0x1) != 0; /* DF_1_NOW = 0x1 */
    }
    
    if (has_bind_now || has_flags_now || has_flags1_now) {
        return 2; /* Full RELRO */
    }
    return 1; /* Partial RELRO */
}

int lief_elf_has_stack_canary(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    /* 检查是否导入__stack_chk_fail或__stack_chk_guard */
    if (wrapper->binary->has_dynamic_symbol("__stack_chk_fail") ||
        wrapper->binary->has_dynamic_symbol("__stack_chk_guard")) {
        return 1;
    }
    return 0;
}

int lief_elf_has_fortify(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    /* 检查是否有FORTIFY相关函数 */
    auto syms = wrapper->binary->dynamic_symbols();
    for (auto it = syms.begin(); it != syms.end(); ++it) {
        std::string name = it->name();
        /* FORTIFY函数通常以__开头并以_chk结尾 */
        if (name.find("_chk") != std::string::npos ||
            name.find("__fortify") != std::string::npos) {
            return 1;
        }
    }
    return 0;
}

int lief_elf_has_rpath(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->has(LIEF::ELF::DynamicEntry::TAG::RPATH) ? 1 : 0;
}

/* ========== Builder配置写入实现 ========== */

int lief_elf_write_with_config(Elf_Binary_Wrapper* wrapper, const char* filepath,
                               int rebuild_hash, int rebuild_symtab) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!filepath) return -1;
    
    try {
        LIEF::ELF::Builder::config_t config;
        config.gnu_hash = (rebuild_hash != 0);
        config.dt_hash = (rebuild_hash != 0);
        config.symtab = (rebuild_symtab != 0);
        config.static_symtab = (rebuild_symtab != 0);
        
        LIEF::ELF::Builder builder(*wrapper->binary, config);
        builder.build();
        builder.write(filepath);
        return 0;
    } catch (...) {
        return -1;
    }
}

/* ========== Android特定信息实现 ========== */

uint32_t lief_elf_android_sdk_version(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    auto* note = wrapper->binary->get(LIEF::ELF::Note::TYPE::ANDROID_IDENT);
    if (!note) return 0;
    
    auto* android_note = static_cast<const LIEF::ELF::AndroidIdent*>(note);
    return android_note->sdk_version();
}

const char* lief_elf_android_ndk_version(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    
    auto* note = wrapper->binary->get(LIEF::ELF::Note::TYPE::ANDROID_IDENT);
    if (!note) return nullptr;
    
    auto* android_note = static_cast<const LIEF::ELF::AndroidIdent*>(note);
    wrapper->name_cache = android_note->ndk_version();
    return wrapper->name_cache.c_str();
}

const char* lief_elf_android_ndk_build_number(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    
    auto* note = wrapper->binary->get(LIEF::ELF::Note::TYPE::ANDROID_IDENT);
    if (!note) return nullptr;
    
    auto* android_note = static_cast<const LIEF::ELF::AndroidIdent*>(note);
    wrapper->name_cache = android_note->ndk_build_number();
    return wrapper->name_cache.c_str();
}

/* ========== ABI信息实现 ========== */

int lief_elf_abi_type(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    auto* note = wrapper->binary->get(LIEF::ELF::Note::TYPE::GNU_ABI_TAG);
    if (!note) return -1;
    
    auto* abi_note = static_cast<const LIEF::ELF::NoteAbi*>(note);
    auto abi_result = abi_note->abi();
    if (!abi_result) return -1;
    
    return static_cast<int>(*abi_result);
}

int lief_elf_abi_version(Elf_Binary_Wrapper* wrapper, 
                         uint32_t* out_major, uint32_t* out_minor, uint32_t* out_patch) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!out_major || !out_minor || !out_patch) return -1;
    
    auto* note = wrapper->binary->get(LIEF::ELF::Note::TYPE::GNU_ABI_TAG);
    if (!note) return -1;
    
    auto* abi_note = static_cast<const LIEF::ELF::NoteAbi*>(note);
    auto version_result = abi_note->version();
    if (!version_result) return -1;
    
    auto& version = *version_result;
    *out_major = version[0];
    *out_minor = version[1];
    *out_patch = version[2];
    return 0;
}

/* ========== Hash表详细信息实现 ========== */

uint32_t lief_elf_gnu_hash_symbol_index(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    auto* gnu_hash = wrapper->binary->gnu_hash();
    if (!gnu_hash) return 0;
    
    return gnu_hash->symbol_index();
}

uint32_t lief_elf_gnu_hash_nb_buckets(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    auto* gnu_hash = wrapper->binary->gnu_hash();
    if (!gnu_hash) return 0;
    
    return gnu_hash->nb_buckets();
}

uint32_t lief_elf_gnu_hash_shift2(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    auto* gnu_hash = wrapper->binary->gnu_hash();
    if (!gnu_hash) return 0;
    
    return gnu_hash->shift2();
}

uint32_t lief_elf_gnu_hash_maskwords(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    auto* gnu_hash = wrapper->binary->gnu_hash();
    if (!gnu_hash) return 0;
    
    return gnu_hash->maskwords();
}

uint32_t lief_elf_sysv_hash_nbucket(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    auto* sysv_hash = wrapper->binary->sysv_hash();
    if (!sysv_hash) return 0;
    
    return sysv_hash->nbucket();
}

uint32_t lief_elf_sysv_hash_nchain(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    auto* sysv_hash = wrapper->binary->sysv_hash();
    if (!sysv_hash) return 0;
    
    return sysv_hash->nchain();
}

/* ========== 符号版本信息实现 ========== */

size_t lief_elf_symbol_version_definitions_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    auto defs = wrapper->binary->symbols_version_definition();
    size_t count = 0;
    for (auto it = defs.begin(); it != defs.end(); ++it, ++count);
    return count;
}

size_t lief_elf_symbol_version_requirements_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    auto reqs = wrapper->binary->symbols_version_requirement();
    size_t count = 0;
    for (auto it = reqs.begin(); it != reqs.end(); ++it, ++count);
    return count;
}

const char* lief_elf_symbol_version_requirement_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    
    auto reqs = wrapper->binary->symbols_version_requirement();
    size_t idx = 0;
    for (auto it = reqs.begin(); it != reqs.end(); ++it, ++idx) {
        if (idx == index) {
            wrapper->name_cache = it->name();
            return wrapper->name_cache.c_str();
        }
    }
    return nullptr;
}

/* ========== 函数辅助功能实现 ========== */

size_t lief_elf_dtor_functions_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->dtor_functions().size();
}

uint64_t lief_elf_dtor_function_address(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    auto dtors = wrapper->binary->dtor_functions();
    if (index >= dtors.size()) return 0;
    
    return dtors[index].address();
}

size_t lief_elf_functions_count(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->functions().size();
}

const char* lief_elf_function_name(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, nullptr);
    
    auto funcs = wrapper->binary->functions();
    if (index >= funcs.size()) return nullptr;
    
    wrapper->name_cache = funcs[index].name();
    return wrapper->name_cache.c_str();
}

uint64_t lief_elf_function_address(Elf_Binary_Wrapper* wrapper, size_t index) {
    CHECK_WRAPPER_RET(wrapper, 0);
    
    auto funcs = wrapper->binary->functions();
    if (index >= funcs.size()) return 0;
    
    return funcs[index].address();
}

int lief_elf_get_function_address(Elf_Binary_Wrapper* wrapper, const char* name, uint64_t* out_address) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name || !out_address) return -1;
    
    auto result = wrapper->binary->get_function_address(name);
    if (!result) return -1;
    
    *out_address = *result;
    return 0;
}

/* ========== 符号索引查找实现 ========== */

int64_t lief_elf_dynsym_idx(Elf_Binary_Wrapper* wrapper, const char* name) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    return wrapper->binary->dynsym_idx(name);
}

int64_t lief_elf_symtab_idx(Elf_Binary_Wrapper* wrapper, const char* name) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!name) return -1;
    
    return wrapper->binary->symtab_idx(name);
}

/* ========== 其他辅助功能实现 ========== */

int lief_elf_has_section_with_offset(Elf_Binary_Wrapper* wrapper, uint64_t offset) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->has_section_with_offset(offset) ? 1 : 0;
}

int lief_elf_has_section_with_va(Elf_Binary_Wrapper* wrapper, uint64_t va) {
    CHECK_WRAPPER_RET(wrapper, 0);
    return wrapper->binary->has_section_with_va(va) ? 1 : 0;
}

int lief_elf_text_section_index(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    auto result = wrapper->binary->get_section_idx(".text");
    if (!result) return -1;
    
    return static_cast<int>(*result);
}

int lief_elf_dynamic_section_index(Elf_Binary_Wrapper* wrapper) {
    CHECK_WRAPPER_RET(wrapper, -1);
    
    auto result = wrapper->binary->get_section_idx(".dynamic");
    if (!result) return -1;
    
    return static_cast<int>(*result);
}

int lief_elf_remove_version_requirement(Elf_Binary_Wrapper* wrapper, const char* libname) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!libname) return -1;
    
    return wrapper->binary->remove_version_requirement(libname) ? 0 : -1;
}

int lief_elf_get_relocated_dynamic_array(Elf_Binary_Wrapper* wrapper, uint64_t tag,
                                         uint64_t** out_values, size_t* out_count) {
    CHECK_WRAPPER_RET(wrapper, -1);
    if (!out_values || !out_count) return -1;
    
    try {
        auto values = wrapper->binary->get_relocated_dynamic_array(
            static_cast<LIEF::ELF::DynamicEntry::TAG>(tag));
        
        if (values.empty()) {
            *out_values = nullptr;
            *out_count = 0;
            return 0;
        }
        
        *out_count = values.size();
        *out_values = (uint64_t*)malloc(sizeof(uint64_t) * values.size());
        if (!*out_values) return -1;
        
        for (size_t i = 0; i < values.size(); i++) {
            (*out_values)[i] = values[i];
        }
        return 0;
    } catch (...) {
        return -1;
    }
}

void lief_elf_free_dynamic_array(uint64_t* values) {
    if (values) free(values);
}
