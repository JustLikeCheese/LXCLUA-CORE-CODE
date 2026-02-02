--[[
    liefx 模块完整功能测试脚本
    测试文件: /storage/emulated/0/Download/liblposixa.so
    
    本脚本展示liefx模块的所有功能用法
]]

local liefx = require("liefx")

-- 测试文件路径
local TEST_FILE = "/storage/emulated/0/Download/liblposixa.so"

-- 格式化输出辅助函数
local function print_header(title)
    print("\n" .. string.rep("=", 60))
    print("  " .. title)
    print(string.rep("=", 60))
end

local function print_section(title)
    print("\n--- " .. title .. " ---")
end

local function print_kv(key, value)
    if value ~= nil then
        print(string.format("  %-30s: %s", key, tostring(value)))
    else
        print(string.format("  %-30s: (nil)", key))
    end
end

-- 格式化地址
local function format_addr(addr)
    if addr then
        return liefx.format_address(addr)
    end
    return "(nil)"
end

-- ============================================================
-- 开始测试
-- ============================================================

print_header("liefx 模块功能完整测试")
print("模块名称: " .. (liefx._NAME or "unknown"))
print("模块版本: " .. (liefx._VERSION or "unknown"))

-- 检查是否为ELF文件
print_section("文件检测")
local is_elf = liefx.is_elf(TEST_FILE)
print_kv("是否为ELF文件", is_elf)
if not is_elf then
    print("错误: 文件不是有效的ELF格式")
    return
end

-- 解析ELF文件
print_section("解析ELF文件")
local elf = liefx.parse(TEST_FILE)
if not elf then
    print("错误: 无法解析ELF文件")
    return
end
print("解析成功: " .. tostring(elf))

-- ============================================================
-- Header信息
-- ============================================================
print_header("ELF Header 信息")

print_kv("入口点", format_addr(elf:entrypoint()))
print_kv("文件类型", elf:type())
print_kv("机器架构", elf:machine())
print_kv("ELF类别", elf:elf_class())
print_kv("字节序", elf:endianness())
print_kv("程序头偏移", format_addr(elf:program_header_offset()))
print_kv("节头偏移", format_addr(elf:section_header_offset()))
print_kv("页大小", elf:page_size())
print_kv("镜像基址", format_addr(elf:imagebase()))
print_kv("虚拟大小", format_addr(elf:virtual_size()))
print_kv("EOF偏移", format_addr(elf:eof_offset()))

-- ============================================================
-- 安全信息
-- ============================================================
print_header("安全检查信息")

print_kv("是否PIE", elf:is_pie())
print_kv("是否NX保护", elf:has_nx())
print_kv("RELRO类型", elf:relro_type() .. " (0=无, 1=部分, 2=完全)")
print_kv("栈金丝雀保护", elf:has_stack_canary())
print_kv("FORTIFY保护", elf:has_fortify())
print_kv("RPATH(安全风险)", elf:has_rpath())

print_section("安全信息汇总")
local sec_info = elf:security_info()
if sec_info then
    print_kv("PIE", sec_info.pie)
    print_kv("NX", sec_info.nx)
    print_kv("RELRO", sec_info.relro)
    print_kv("Canary", sec_info.canary)
    print_kv("FORTIFY", sec_info.fortify)
    print_kv("RPATH", sec_info.rpath)
end

-- ============================================================
-- 解释器
-- ============================================================
print_header("解释器信息")

print_kv("是否有解释器", elf:has_interpreter())
if elf:has_interpreter() then
    print_kv("解释器路径", elf:interpreter())
end

-- ============================================================
-- Android信息
-- ============================================================
print_header("Android特定信息")

print_kv("是否针对Android", elf:is_targeting_android())
print_kv("SDK版本", elf:android_sdk_version())
print_kv("NDK版本", elf:android_ndk_version())
print_kv("NDK构建号", elf:android_ndk_build_number())

local android_info = elf:android_info()
if android_info then
    print_section("Android完整信息")
    for k, v in pairs(android_info) do
        print_kv(k, v)
    end
end

-- ============================================================
-- ABI信息
-- ============================================================
print_header("ABI信息")

print_kv("ABI类型", elf:abi_type())
local abi_ver = elf:abi_version()
if abi_ver then
    print_kv("ABI版本", abi_ver.string)
    print_kv("  Major", abi_ver.major)
    print_kv("  Minor", abi_ver.minor)
    print_kv("  Patch", abi_ver.patch)
end

-- ============================================================
-- 数量统计
-- ============================================================
print_header("数量统计")

local counts = elf:counts()
if counts then
    print_kv("节数量", counts.sections)
    print_kv("段数量", counts.segments)
    print_kv("动态符号数量", counts.dynamic_symbols)
    print_kv("symtab符号数量", counts.symtab_symbols)
    print_kv("重定位数量", counts.relocations)
    print_kv("动态条目数量", counts.dynamic_entries)
    print_kv("库依赖数量", counts.libraries)
    print_kv("Note数量", counts.notes)
end

-- ============================================================
-- 节(Section)信息
-- ============================================================
print_header("节(Section)信息")

local sections = elf:sections()
print("共 " .. #sections .. " 个节:")
print(string.format("  %-4s %-25s %-12s %-12s %-10s", "序号", "名称", "类型", "大小", "虚拟地址"))
print("  " .. string.rep("-", 70))
for i, sec in ipairs(sections) do
    print(string.format("  %-4d %-25s %-12s %-12s %-10s",
        i, sec.name or "", sec.type or 0, sec.size or 0, format_addr(sec.virtual_address)))
end

print_section("特殊节索引")
print_kv(".text节索引", elf:text_section_index())
print_kv(".dynamic节索引", elf:dynamic_section_index())

print_section("节查询")
print_kv("是否有.text节", elf:has_section(".text"))
print_kv("是否有.rodata节", elf:has_section(".rodata"))
print_kv("是否有.bss节", elf:has_section(".bss"))

local text_sec = elf:get_section(".text")
if text_sec then
    print_section(".text节详情")
    print_kv("名称", text_sec.name)
    print_kv("类型", text_sec.type)
    print_kv("虚拟地址", format_addr(text_sec.virtual_address))
    print_kv("大小", text_sec.size)
    print_kv("偏移", format_addr(text_sec.offset))
end

-- ============================================================
-- 段(Segment)信息
-- ============================================================
print_header("段(Segment)信息")

local segments = elf:segments()
print("共 " .. #segments .. " 个段:")
print(string.format("  %-4s %-12s %-12s %-12s %-12s %-10s", "序号", "类型", "标志", "文件大小", "内存大小", "虚拟地址"))
print("  " .. string.rep("-", 75))
for i, seg in ipairs(segments) do
    print(string.format("  %-4d %-12s %-12s %-12s %-12s %-10s",
        i, seg.type or 0, seg.flags or 0, seg.file_size or 0, seg.virtual_size or 0, format_addr(seg.virtual_address)))
end

-- ============================================================
-- 动态符号
-- ============================================================
print_header("动态符号信息")

local dyn_syms = elf:dynamic_symbols()
print("共 " .. #dyn_syms .. " 个动态符号 (显示前20个):")
print(string.format("  %-4s %-40s %-12s %-8s %-8s", "序号", "名称", "值", "大小", "类型"))
print("  " .. string.rep("-", 80))
for i = 1, math.min(20, #dyn_syms) do
    local sym = dyn_syms[i]
    print(string.format("  %-4d %-40s %-12s %-8s %-8s",
        i, (sym.name or ""):sub(1, 40), format_addr(sym.value), sym.size or 0, sym.type or 0))
end
if #dyn_syms > 20 then
    print("  ... 还有 " .. (#dyn_syms - 20) .. " 个符号")
end

print_section("符号查询")
-- 尝试查找常见函数
local test_symbols = {"open", "close", "read", "write", "malloc", "free", "printf"}
for _, sym_name in ipairs(test_symbols) do
    local has = elf:has_dynamic_symbol(sym_name)
    if has then
        local sym = elf:get_dynamic_symbol(sym_name)
        if sym then
            print(string.format("  %-15s 存在, 值=%s, 大小=%d", sym_name, format_addr(sym.value), sym.size or 0))
        end
    end
end

print_section("符号索引查找")
print_kv("open的dynsym索引", elf:dynsym_idx("open"))
print_kv("close的dynsym索引", elf:dynsym_idx("close"))

-- ============================================================
-- 导入/导出符号
-- ============================================================
print_header("导入/导出符号")

print_section("导入符号 (前10个)")
local imported = elf:imported_symbols()
for i = 1, math.min(10, #imported) do
    print("  " .. i .. ". " .. imported[i])
end
if #imported > 10 then
    print("  ... 还有 " .. (#imported - 10) .. " 个")
end

print_section("导出符号 (前10个)")
local exported = elf:exported_symbols()
for i = 1, math.min(10, #exported) do
    local sym = exported[i]
    print(string.format("  %d. %s @ %s", i, sym.name or "", format_addr(sym.value)))
end
if #exported > 10 then
    print("  ... 还有 " .. (#exported - 10) .. " 个")
end

-- ============================================================
-- 导入/导出函数
-- ============================================================
print_header("导入/导出函数")

print_section("导入函数 (前10个)")
local imp_funcs = elf:imported_functions()
for i = 1, math.min(10, #imp_funcs) do
    print("  " .. i .. ". " .. (imp_funcs[i].name or imp_funcs[i]))
end
if #imp_funcs > 10 then
    print("  ... 还有 " .. (#imp_funcs - 10) .. " 个")
end

print_section("导出函数 (前10个)")
local exp_funcs = elf:exported_functions()
for i = 1, math.min(10, #exp_funcs) do
    local f = exp_funcs[i]
    if type(f) == "table" then
        print(string.format("  %d. %s @ %s", i, f.name or "", format_addr(f.address)))
    else
        print("  " .. i .. ". " .. tostring(f))
    end
end
if #exp_funcs > 10 then
    print("  ... 还有 " .. (#exp_funcs - 10) .. " 个")
end

-- ============================================================
-- 构造/析构函数
-- ============================================================
print_header("构造/析构函数")

print_section("构造函数(ctor)")
local ctors = elf:ctor_functions()
for i, addr in ipairs(ctors) do
    print(string.format("  %d. %s", i, format_addr(addr)))
end

print_section("析构函数(dtor)")
local dtors = elf:dtor_functions()
for i, addr in ipairs(dtors) do
    print(string.format("  %d. %s", i, format_addr(addr)))
end

-- ============================================================
-- 所有函数
-- ============================================================
print_header("所有函数 (前15个)")

local all_funcs = elf:functions()
print("共 " .. #all_funcs .. " 个函数:")
for i = 1, math.min(15, #all_funcs) do
    local f = all_funcs[i]
    print(string.format("  %d. %-40s @ %s", i, (f.name or ""):sub(1, 40), format_addr(f.address)))
end
if #all_funcs > 15 then
    print("  ... 还有 " .. (#all_funcs - 15) .. " 个")
end

print_section("按名称获取函数地址")
local func_names = {"open", "close", "read"}
for _, name in ipairs(func_names) do
    local addr = elf:get_function_address(name)
    print_kv(name, addr and format_addr(addr) or "(未找到)")
end

-- ============================================================
-- 重定位信息
-- ============================================================
print_header("重定位信息")

local relocs = elf:relocations()
print("共 " .. #relocs .. " 个重定位 (显示前10个):")
print(string.format("  %-4s %-14s %-8s %-40s", "序号", "地址", "类型", "符号"))
print("  " .. string.rep("-", 70))
for i = 1, math.min(10, #relocs) do
    local r = relocs[i]
    print(string.format("  %-4d %-14s %-8s %-40s",
        i, format_addr(r.address), r.type or 0, (r.symbol or ""):sub(1, 40)))
end
if #relocs > 10 then
    print("  ... 还有 " .. (#relocs - 10) .. " 个")
end

print_section("重定位查询")
-- 尝试按符号查找重定位
local reloc = elf:get_relocation_by_symbol("open")
if reloc then
    print("  open的重定位:")
    print_kv("    地址", format_addr(reloc.address))
    print_kv("    类型", reloc.type)
    print_kv("    加数", reloc.addend)
end

-- ============================================================
-- 动态条目
-- ============================================================
print_header("动态条目信息")

local dyn_entries = elf:dynamic_entries()
print("共 " .. #dyn_entries .. " 个动态条目:")
for i, entry in ipairs(dyn_entries) do
    print(string.format("  %d. tag=%-10s value=%s",
        i, entry.tag or 0, format_addr(entry.value)))
end

print_section("动态条目查询")
print_kv("是否有NEEDED", elf:has_dynamic_entry(1))
print_kv("是否有SONAME", elf:has_dynamic_entry(14))
print_kv("是否有INIT_ARRAY", elf:has_dynamic_entry(25))

-- ============================================================
-- 库依赖
-- ============================================================
print_header("库依赖信息")

local libs = elf:libraries()
print("共 " .. #libs .. " 个库依赖:")
for i, lib in ipairs(libs) do
    print("  " .. i .. ". " .. lib)
end

print_section("库依赖查询")
print_kv("是否依赖libc.so", elf:has_library("libc.so"))
print_kv("是否依赖libm.so", elf:has_library("libm.so"))
print_kv("是否依赖libdl.so", elf:has_library("libdl.so"))

-- ============================================================
-- SONAME和RUNPATH
-- ============================================================
print_header("SONAME和RUNPATH")

print_kv("SONAME", elf:soname())
print_kv("RUNPATH", elf:runpath())

-- ============================================================
-- Hash表信息
-- ============================================================
print_header("Hash表信息")

print_kv("是否有GNU Hash", elf:has_gnu_hash())
print_kv("是否有调试信息", elf:has_debug_info())

print_section("GNU Hash详情")
local gnu_hash = elf:gnu_hash_info()
if gnu_hash then
    print_kv("符号起始索引", gnu_hash.symbol_index)
    print_kv("桶数量", gnu_hash.nb_buckets)
    print_kv("Shift2", gnu_hash.shift2)
    print_kv("Maskwords", gnu_hash.maskwords)
end

print_section("SYSV Hash详情")
local sysv_hash = elf:sysv_hash_info()
if sysv_hash then
    print_kv("桶数量", sysv_hash.nbucket)
    print_kv("链数量", sysv_hash.nchain)
end

print_section("GNU Hash符号检查")
print_kv("open可能存在", elf:gnu_hash_check("open"))
print_kv("nonexistent可能存在", elf:gnu_hash_check("nonexistent_symbol_12345"))

-- ============================================================
-- 符号版本信息
-- ============================================================
print_header("符号版本信息")

print_kv("版本定义数量", elf:symbol_version_definitions_count())

print_section("符号版本需求")
local ver_reqs = elf:symbol_version_requirements()
for i, req in ipairs(ver_reqs) do
    print("  " .. i .. ". " .. req)
end

-- ============================================================
-- 虚拟地址辅助
-- ============================================================
print_header("虚拟地址辅助功能")

print_kv("下一个可用虚拟地址", format_addr(elf:next_virtual_address()))
print_kv("节表最后偏移", format_addr(elf:last_offset_section()))
print_kv("段表最后偏移", format_addr(elf:last_offset_segment()))

print_section("地址转换")
local test_va = elf:entrypoint()
if test_va and test_va > 0 then
    local offset = elf:va_to_offset(test_va)
    print_kv("入口点VA", format_addr(test_va))
    print_kv("对应偏移", offset and format_addr(offset) or "(转换失败)")
    
    if offset then
        local va_back = elf:offset_to_va(offset)
        print_kv("偏移转回VA", va_back and format_addr(va_back) or "(转换失败)")
    end
end

print_section("地址查询")
print_kv("偏移0x1000是否有节", elf:has_section_with_offset(0x1000))
print_kv("VA 0x1000是否有节", elf:has_section_with_va(0x1000))

-- ============================================================
-- Note信息
-- ============================================================
print_header("Note信息")

local notes = elf:notes()
print("共 " .. #notes .. " 个Note:")
for i, note in ipairs(notes) do
    print(string.format("  %d. name=%s, type=%s", i, note.name or "", note.type or 0))
end

-- ============================================================
-- Overlay信息
-- ============================================================
print_header("Overlay信息")

print_kv("是否有Overlay", elf:has_overlay())
if elf:has_overlay() then
    local overlay = elf:overlay()
    print_kv("Overlay大小", overlay and #overlay or 0)
end

-- ============================================================
-- 字符串提取
-- ============================================================
print_header("字符串提取 (最小5字符)")

local strings = elf:strings(5)
print("共提取 " .. #strings .. " 个字符串 (显示前20个):")
for i = 1, math.min(20, #strings) do
    local s = strings[i]
    if #s > 60 then
        s = s:sub(1, 57) .. "..."
    end
    print("  " .. i .. ". " .. s)
end
if #strings > 20 then
    print("  ... 还有 " .. (#strings - 20) .. " 个")
end

-- ============================================================
-- 重定位动态数组
-- ============================================================
print_header("重定位动态数组")

print_section("INIT_ARRAY (tag=25)")
local init_arr = elf:get_relocated_dynamic_array(25)
for i, addr in ipairs(init_arr) do
    print(string.format("  %d. %s", i, format_addr(addr)))
end

print_section("FINI_ARRAY (tag=26)")
local fini_arr = elf:get_relocated_dynamic_array(26)
for i, addr in ipairs(fini_arr) do
    print(string.format("  %d. %s", i, format_addr(addr)))
end

-- ============================================================
-- 读取内容
-- ============================================================
print_header("内容读取测试")

local entry = elf:entrypoint()
if entry and entry > 0 then
    print_section("从入口点读取16字节")
    local content = elf:read_from_va(entry, 16)
    if content then
        local hex = ""
        for i = 1, math.min(16, #content) do
            hex = hex .. string.format("%02X ", content:byte(i))
        end
        print("  " .. hex)
    end
end

-- ============================================================
-- 反汇编功能
-- ============================================================
print_header("反汇编功能测试")

print_section("从入口点反汇编10条指令")
local entry_addr = elf:entrypoint()
print_kv("入口点地址", format_addr(entry_addr))
if entry_addr and entry_addr > 0 then
    -- size参数是字节数，ARM64每条指令4字节，所以40字节约10条指令
    local insts, err = elf:disassemble(entry_addr, 100)
    if insts and #insts > 0 then
        print(string.format("  %-14s %-10s %-30s", "地址", "助记符", "操作数"))
        print("  " .. string.rep("-", 60))
        for i, inst in ipairs(insts) do
            print(string.format("  %-14s %-10s %-30s",
                format_addr(inst.address),
                inst.mnemonic or "",
                inst.operands or ""))
        end
    else
        print("  反汇编失败: " .. (err or "未知错误"))
    end
else
    print("  (无有效入口点)")
end

print_section("反汇编指定缓冲区")
-- 使用一些ARM64示例字节码 (MOV X0, #0; RET)
local test_bytes = string.char(0x00, 0x00, 0x80, 0xD2, 0xC0, 0x03, 0x5F, 0xD6)
local buf_insts, buf_err = elf:disassemble_buffer(test_bytes, 0x1000)
if buf_insts and #buf_insts > 0 then
    print("  测试字节码反汇编结果:")
    for i, inst in ipairs(buf_insts) do
        print(string.format("    %s: %s %s", 
            format_addr(inst.address), 
            inst.mnemonic or "", 
            inst.operands or ""))
    end
else
    print("  缓冲区反汇编失败: " .. (buf_err or "未知错误"))
end

print_section("按符号名反汇编")
-- 尝试反汇编一些常见符号
local sym_names = {"open", "close", "read"}
for _, sym_name in ipairs(sym_names) do
    if elf:has_dynamic_symbol(sym_name) then
        local sym_insts = elf:disassemble_symbol(sym_name)
        if sym_insts and #sym_insts > 0 then
            print("  " .. sym_name .. " 函数 (前5条):")
            for i = 1, math.min(5, #sym_insts) do
                local inst = sym_insts[i]
                print(string.format("    %s: %s %s",
                    format_addr(inst.address),
                    inst.mnemonic or "",
                    inst.operands or ""))
            end
        end
        break
    end
end

-- ============================================================
-- 汇编功能
-- ============================================================
print_header("汇编功能测试")

print_section("汇编指令到字节码")
-- 尝试汇编一条简单指令
local asm_code = "mov x0, #0"
local asm_bytes, asm_err = elf:assemble(0x1000, asm_code)
if asm_bytes and #asm_bytes > 0 then
    local hex = ""
    for i = 1, #asm_bytes do
        hex = hex .. string.format("%02X ", asm_bytes:byte(i))
    end
    print("  指令: " .. asm_code)
    print("  字节码: " .. hex)
else
    print("  汇编失败: " .. (asm_err or "不支持当前架构"))
end

print_section("多条指令汇编")
local multi_asm = [[
    mov x0, #1
    mov x1, #2
    add x0, x0, x1
    ret
]]
local multi_bytes, multi_err = elf:assemble(0x1000, multi_asm)
if multi_bytes and #multi_bytes > 0 then
    print("  汇编 " .. #multi_bytes .. " 字节")
    local hex = ""
    for i = 1, math.min(32, #multi_bytes) do
        hex = hex .. string.format("%02X ", multi_bytes:byte(i))
        if i % 16 == 0 then hex = hex .. "\n  " end
    end
    print("  " .. hex)
else
    print("  多指令汇编失败: " .. (multi_err or "不支持当前架构"))
end

-- ============================================================
-- symtab符号 (如果有)
-- ============================================================
print_header("Symtab符号信息")

local symtab = elf:symtab_symbols()
print("共 " .. #symtab .. " 个symtab符号")
if #symtab > 0 then
    print("(显示前10个):")
    for i = 1, math.min(10, #symtab) do
        local sym = symtab[i]
        print(string.format("  %d. %-40s @ %s", i, (sym.name or ""):sub(1, 40), format_addr(sym.value)))
    end
end

-- ============================================================
-- 节/段内容读取
-- ============================================================
print_header("节/段内容读取")

print_section("读取.text节内容 (前32字节)")
local text_content = elf:section_content(".text")
if text_content and #text_content > 0 then
    local hex = ""
    for i = 1, math.min(32, #text_content) do
        hex = hex .. string.format("%02X ", text_content:byte(i))
        if i % 16 == 0 then hex = hex .. "\n  " end
    end
    print("  " .. hex)
    print("  总大小: " .. #text_content .. " 字节")
else
    print("  (无法读取)")
end

print_section("读取LOAD段内容 (前32字节)")
local seg_content = elf:segment_content(0) -- 第一个段
if seg_content and #seg_content > 0 then
    local hex = ""
    for i = 1, math.min(32, #seg_content) do
        hex = hex .. string.format("%02X ", seg_content:byte(i))
        if i % 16 == 0 then hex = hex .. "\n  " end
    end
    print("  " .. hex)
    print("  总大小: " .. #seg_content .. " 字节")
else
    print("  (无法读取)")
end

-- ============================================================
-- 节/段查找
-- ============================================================
print_header("节/段查找功能")

print_section("通过偏移查找节")
local sec_by_offset = elf:section_from_offset(0x1000)
if sec_by_offset then
    print_kv("偏移0x1000处的节", sec_by_offset.name)
end

print_section("通过虚拟地址查找节")
local test_va = elf:entrypoint()
if test_va and test_va > 0 then
    local sec_by_va = elf:section_from_va(test_va)
    if sec_by_va then
        print_kv("入口点所在节", sec_by_va.name)
    end
end

print_section("通过偏移查找段")
local seg_by_offset = elf:segment_from_offset(0x1000)
if seg_by_offset then
    print_kv("偏移0x1000处的段类型", seg_by_offset.type)
end

print_section("通过虚拟地址查找段")
if test_va and test_va > 0 then
    local seg_by_va = elf:segment_from_va(test_va)
    if seg_by_va then
        print_kv("入口点所在段类型", seg_by_va.type)
    end
end

-- ============================================================
-- 获取原始字节
-- ============================================================
print_header("获取原始字节(raw)")

local raw_bytes = elf:raw()
if raw_bytes and #raw_bytes > 0 then
    print("  原始字节大小: " .. #raw_bytes .. " 字节")
    print("  ELF魔数验证: " .. 
        string.format("%02X %02X %02X %02X", 
            raw_bytes:byte(1), raw_bytes:byte(2), 
            raw_bytes:byte(3), raw_bytes:byte(4)))
else
    print("  (获取失败)")
end

-- ============================================================
-- Patch操作演示 (仅展示用法，不实际修改)
-- ============================================================
print_header("Patch操作演示")

print([[
  以下为Patch操作的用法示例(不实际执行):
  
  -- 通过虚拟地址patch字节
  elf:patch(0x1000, "\x90\x90\x90\x90")  -- 写入4个NOP
  
  -- patch单个值
  elf:patch_value(0x1000, 0x12345678, 4)  -- 写入4字节值
  
  -- patch PLT/GOT表
  elf:patch_pltgot("open", 0xDEADBEEF)  -- 重定向open函数
  
  -- 使用汇编patch
  elf:assemble_patch(0x1000, "nop; nop; ret")
]])

-- ============================================================
-- 修改操作演示 (仅展示用法，不实际修改)
-- ============================================================
print_header("修改操作演示")

print([[
  以下为修改操作的用法示例(不实际执行):
  
  -- 设置入口点
  elf:set_entrypoint(0x1000)
  
  -- 设置解释器
  elf:set_interpreter("/lib64/ld-linux-x86-64.so.2")
  
  -- 设置SONAME
  elf:set_soname("mylib.so")
  
  -- 设置RUNPATH
  elf:set_runpath("/usr/local/lib")
  
  -- 修改节内容
  elf:set_section_content(".text", new_bytes)
  
  -- 修改段内容
  elf:set_segment_content(0, new_bytes)
  
  -- 修改节属性
  elf:modify_section(".text", {
      virtual_address = 0x1000,
      flags = liefx.SHF.ALLOC + liefx.SHF.EXECINSTR
  })
  
  -- 修改段属性
  elf:modify_segment(0, {
      virtual_address = 0x1000,
      flags = liefx.PF.R + liefx.PF.X
  })
  
  -- 设置Overlay
  elf:set_overlay("custom data here")
]])

-- ============================================================
-- 添加操作演示 (仅展示用法，不实际修改)
-- ============================================================
print_header("添加操作演示")

print([[
  以下为添加操作的用法示例(不实际执行):
  
  -- 添加新节
  elf:add_section({
      name = ".mysection",
      type = liefx.SHT.PROGBITS,
      flags = liefx.SHF.ALLOC,
      content = "section data"
  })
  
  -- 添加新段
  elf:add_segment({
      type = liefx.PT.LOAD,
      flags = liefx.PF.R + liefx.PF.W,
      content = "segment data"
  })
  
  -- 添加动态符号
  elf:add_dynamic_symbol({
      name = "my_symbol",
      value = 0x1000,
      size = 100,
      type = liefx.STT.FUNC,
      binding = liefx.STB.GLOBAL
  })
  
  -- 添加symtab符号
  elf:add_symtab_symbol({
      name = "local_symbol",
      value = 0x2000,
      size = 50,
      type = liefx.STT.OBJECT,
      binding = liefx.STB.LOCAL
  })
  
  -- 添加库依赖
  elf:add_library("libcustom.so")
  
  -- 导出符号
  elf:export_symbol("exported_func", 0x3000)
  
  -- 添加重定位
  elf:add_dynamic_relocation({
      address = 0x4000,
      type = 1,
      symbol = "target_sym",
      addend = 0
  })
  
  elf:add_pltgot_relocation({
      address = 0x5000,
      type = 2,
      symbol = "plt_target"
  })
]])

-- ============================================================
-- 删除操作演示 (仅展示用法，不实际修改)
-- ============================================================
print_header("删除操作演示")

print([[
  以下为删除操作的用法示例(不实际执行):
  
  -- 删除节
  elf:remove_section(".debug_info", true)  -- true=清零内容
  
  -- 删除段
  elf:remove_segment(5)  -- 按索引删除
  
  -- 删除动态符号
  elf:remove_dynamic_symbol("unused_symbol")
  
  -- 删除symtab符号
  elf:remove_symtab_symbol("debug_symbol")
  
  -- 删除所有同名符号
  elf:remove_symbol("old_symbol")
  
  -- 删除库依赖
  elf:remove_library("libold.so")
  
  -- 删除动态条目
  elf:remove_dynamic_entry(liefx.DT.DEBUG)
  
  -- 删除符号版本需求
  elf:remove_version_requirement("libc.so.6")
  
  -- strip (删除所有调试符号)
  elf:strip()
]])

-- ============================================================
-- 写入操作演示 (仅展示用法，不实际修改)
-- ============================================================
print_header("写入操作演示")

print([[
  以下为写入操作的用法示例(不实际执行):
  
  -- 普通写入
  elf:write("/path/to/output.so")
  
  -- 带配置写入
  elf:write_config("/path/to/output.so", {
      rebuild_hash = true,     -- 重建hash表
      rebuild_symtab = true    -- 重建符号表
  })
]])

-- ============================================================
-- 常量值参考
-- ============================================================
print_header("常量值参考")

print_section("ELF类型 (E_TYPE)")
print("  NONE=0, REL=1, EXEC=2, DYN=3, CORE=4")

print_section("机器架构 (ARCH)")
print("  NONE=0, I386=3, X86_64=62, ARM=40, AARCH64=183, MIPS=8, RISCV=243")

print_section("节类型 (SHT)")
print("  NULL=0, PROGBITS=1, SYMTAB=2, STRTAB=3, RELA=4, HASH=5")
print("  DYNAMIC=6, NOTE=7, NOBITS=8, REL=9, DYNSYM=11")

print_section("节标志 (SHF)")
print("  NONE=0x0, WRITE=0x1, ALLOC=0x2, EXECINSTR=0x4")

print_section("段类型 (PT)")
print("  NULL=0, LOAD=1, DYNAMIC=2, INTERP=3, NOTE=4, PHDR=6")
print("  GNU_EH_FRAME=0x6474e550, GNU_STACK=0x6474e551, GNU_RELRO=0x6474e552")

print_section("段标志 (PF)")
print("  NONE=0, X=1, W=2, R=4")

print_section("符号绑定 (STB)")
print("  LOCAL=0, GLOBAL=1, WEAK=2, GNU_UNIQUE=10")

print_section("符号类型 (STT)")
print("  NOTYPE=0, OBJECT=1, FUNC=2, SECTION=3, FILE=4, COMMON=5, TLS=6")

print_section("动态标签 (DT)")
print("  NULL=0, NEEDED=1, PLTRELSZ=2, PLTGOT=3, HASH=4, STRTAB=5")
print("  SYMTAB=6, RELA=7, INIT=12, FINI=13, SONAME=14, RPATH=15")
print("  INIT_ARRAY=25, FINI_ARRAY=26, RUNPATH=29, FLAGS=30")

-- ============================================================
-- 完成
-- ============================================================
print_header("测试完成")
print("所有liefx功能测试完毕!")
print("共测试 130+ 个API方法")

-- 释放资源
elf:destroy()
print("资源已释放")
