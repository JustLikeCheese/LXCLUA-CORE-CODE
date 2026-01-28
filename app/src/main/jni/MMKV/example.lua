-- MMKV 完整使用示例

print("========== MMKV 使用示例 ==========\n")

local mmkv = require "mmkv"

-- 1. 初始化
mmkv.init()

-- 2. 打开实例
local id = "my_app_data"
local ok, msg = mmkv.open(id)
print("打开实例:", ok, msg, "\n")

-- ========== 存储数据 ==========

-- 字符串
mmkv.putStr(id, "username", "张三")
mmkv.putStr(id, "password", "123456")

-- 整数
mmkv.putInt(id, "age", 25)
mmkv.putInt(id, "score", 100)
mmkv.putInt(id, "level", 10)

-- 浮点数
mmkv.putFloat(id, "price", 19.99)
mmkv.putDouble(id, "lat", 39.9042)
mmkv.putDouble(id, "lng", 116.4074)

-- 布尔值
mmkv.put(id, "is_vip", true)
mmkv.put(id, "is_login", false)

print("=== 存储完成 ===\n")

-- ========== 读取数据 ==========

print("=== 读取数据 ===")
print("username:", mmkv.getStr(id, "username", ""))
print("age:", mmkv.getInt(id, "age", 0))
print("price:", mmkv.getInt(id, "price", 0))  -- getInt 返回整数
print("lat:", mmkv.getStr(id, "lat", "0"))    -- getStr 返回字符串
print("is_vip:", mmkv.get(id, "is_vip", false))
print("\n")

-- ========== 检查键是否存在 ==========

print("=== 检查键 ===")
print("has username:", mmkv.has(id, "username"))
print("has not_exist:", mmkv.has(id, "not_exist"))
print("\n")

-- ========== 获取所有键 ==========

print("=== 所有键 ===")
local keys = mmkv.keys(id)
for i, k in ipairs(keys) do
    print("  ", i, k)
end
print("\n")

-- ========== 统计信息 ==========

print("=== 统计信息 ===")
print("键数量:", mmkv.size(id))
print("总大小:", mmkv.total(id))
print("已用大小:", mmkv.used(id))
print("\n")

-- ========== 删除数据 ==========

print("=== 删除数据 ===")
mmkv.del(id, "password")
print("删除 password 后 has:", mmkv.has(id, "password"))

-- 批量删除
mmkv.dels(id, {"age", "level"})
print("批量删除后 size:", mmkv.size(id))
print("\n")

-- ========== 清除所有 ==========

print("=== 清除所有 ===")
mmkv.clear(id, false)
print("clear 后 size:", mmkv.size(id))
print("\n")

-- ========== 重新存储测试 ==========

mmkv.putStr(id, "test", "hello")
print("重新测试 getStr:", mmkv.getStr(id, "test", ""))

-- ========== 关闭和重新打开 ==========

print("\n=== 关闭和重新打开 ===")
mmkv.close(id)
local id2, ok2, msg2 = mmkv.open(id)
print("重新打开:", ok2, msg2)
print("重新打开后 getStr:", mmkv.getStr(id2, "test", ""))

-- ========== 删除整个实例 ==========

print("\n=== 删除实例 ===")
mmkv.remove(id)
print("删除实例后 exists:", mmkv.exists(id))

-- ========== 退出 ==========

mmkv.exit()

print("\n========== 测试完成 ==========")
