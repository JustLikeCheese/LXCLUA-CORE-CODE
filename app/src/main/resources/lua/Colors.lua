-- 作者 Aqora
-- QQ 2241056127

local _M = {}
local bindClass = luajava.bindClass
local MDC_R = bindClass "com.google.android.material.R"
local MaterialColors = bindClass "com.google.android.material.color.MaterialColors"
local android_R = bindClass "android.R"

-- 常量定义
local DEFAULT_STATE = android_R.attr.state_enabled

-- 安全获取表中键值
_M.get = function(table, key)
  local val
  pcall(function() val = table[key] end)
  return val
end

-- 获取属性值
local getAttr = function(name)
  return _M.get(MDC_R.attr, name) or _M.get(android_R.attr, name)
end

-- 元表配置
return setmetatable({}, {
  __index = function(self, key)
    return _M.get(MaterialColors, key)
    or MaterialColors.getColor(this, getAttr(key), DEFAULT_STATE)
  end
})