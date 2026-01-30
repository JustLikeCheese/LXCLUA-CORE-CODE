/**
 * @file lief_elf_wrapper.cpp
 * @brief LIEF ELF C++ API的C语言wrapper实现
 * @description 为LIEF的C++ API提供C语言接口，用于Lua绑定
 * @author DifierLine
 * @date 2026-01-30
 */

#include "lief_elf_wrapper.hpp"

#include <LIEF/ELF.hpp>
#include <memory>
#include <vector>
#include <string>

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
