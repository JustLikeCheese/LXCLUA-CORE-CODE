local L = luajava.bindClass("com.difierline.luajitcore.luajit.LuaState")

local M = {}



local function _stack_to_lua(l, idx)
    local abs_idx = idx < 0 and l.getTop() + idx + 1 or idx
    if not l.isTable(abs_idx) then
        local t = l.type(abs_idx)
        if t == l.LUA_TSTRING then return l.toString(abs_idx) end
        if t == l.LUA_TINTEGER then return l.toInteger(abs_idx) end
        if t == l.LUA_TNUMBER then return l.toNumber(abs_idx) end
        if t == l.LUA_TBOOLEAN then return l.toBoolean(abs_idx) end
        return nil
    end

    local tbl = {}
    l.pushNil()
    while l.next(abs_idx) ~= 0 do
        local k = _stack_to_lua(l, -2)
        local v = _stack_to_lua(l, -1)
        if k ~= nil then tbl[k] = v end
        l.pop(1)
    end
    return tbl
end



local function _get_lua_error(l, err_code)
    l.Lwhere(1)
    local where = l.toString(-1)
    l.pop(1)
    local err_msg = l.toString(-1)
    l.pop(1)
    local err_types = {
        [l.LUA_ERRSYNTAX] = "syntax error",
        [l.LUA_ERRRUN] = "runtime error",
        [l.LUA_ERRMEM] = "memory error",
        [l.LUA_ERRGCMM] = "gc error",
        [l.LUA_ERRERR] = "error handler error"
    }
    return string.format("[%s] %s: %s", err_types[err_code] or "unknown error", where or "unknown pos", err_msg or "no error message")
end

function M.loadstring(lua_code)
    local ret = nil
    local l = luajava.new(L)
    local p = l.getPointer()

    if p == 0 or l.isClosed() then
        l.close()
        error("invalid or closed Lua state")
    end

    

    local load_ret = l.LloadString(lua_code)
    if load_ret ~= 0 then
        local err = _get_lua_error(l, load_ret)
        l.close()
        error(err)
    end

    

    local run_ret = l.LdoString(lua_code)
    if run_ret ~= 0 then
        local err = _get_lua_error(l, run_ret)
        l.close()
        error(err)
    end

    local top = l.getTop()
    if top > 0 then
        ret = _stack_to_lua(l, -1)
    end
    l.pop(l.getTop())
    l.close()
    return ret
end

function M.dofile(luac_filename)
    local ret = nil
    local luac_path = activity.getLuaDir() .. "/" .. luac_filename
    local l = luajava.new(L)
    local p = l.getPointer()

    if p == 0 or l.isClosed() then
        l.close()
        error("invalid or closed Lua state")
    end

    local exec_str = string.format([[
        local f = loadfile("%s")
        if not f then error("file load failed: %s") end
        return f()
    ]], luac_path:gsub("\\", "\\\\"):gsub('"', '\\"'), luac_path:gsub("\\", "\\\\"):gsub('"', '\\"'))

    

    local load_ret = l.LloadString(exec_str)
    if load_ret ~= 0 then
        local err = _get_lua_error(l, load_ret)
        l.close()
        error(string.format("dofile syntax error: %s", err))
    end

    

    local run_ret = l.LdoString(exec_str)
    if run_ret ~= 0 then
        local err = _get_lua_error(l, run_ret)
        l.close()
        error(string.format("dofile runtime error: %s", err))
    end

    local top = l.getTop()
    if top > 0 then
        ret = _stack_to_lua(l, -1)
    end
    l.pop(l.getTop())
    l.close()
    return ret
end

function M.loadfile(full_path)
    local ret = nil
    local l = luajava.new(L)
    local p = l.getPointer()

    if p == 0 or l.isClosed() then
        l.close()
        error("invalid or closed Lua state")
    end

    local exec_str = string.format([[
        local f = loadfile("%s")
        if not f then error("file load failed: %s") end
        return f()
    ]], full_path:gsub("\\", "\\\\"):gsub('"', '\\"'), full_path:gsub("\\", "\\\\"):gsub('"', '\\"'))

    

    local load_ret = l.LloadString(exec_str)
    if load_ret ~= 0 then
        local err = _get_lua_error(l, load_ret)
        l.close()
        error(string.format("loadfile syntax error: %s", err))
    end

    

    local run_ret = l.LdoString(exec_str)
    if run_ret ~= 0 then
        local err = _get_lua_error(l, run_ret)
        l.close()
        error(string.format("loadfile runtime error: %s", err))
    end

    local top = l.getTop()
    if top > 0 then
        ret = _stack_to_lua(l, -1)
    end
    l.pop(l.getTop())
    l.close()
    return ret
end

function M.setglobal(var_name, var_value)
    local l = luajava.new(L)
    local p = l.getPointer()
    if p == 0 or l.isClosed() then
        l.close()
        error("invalid or closed Lua state")
    end

    local function _push_val(v)
        local t = type(v)
        if t == "table" then
            l.newTable()
            for k, val in pairs(v) do
                l.pushString(tostring(k))
                _push_val(val)
                l.rawSet(-3)
            end
        elseif t == "string" then 
            l.pushString(v)
        elseif t == "number" then 
            l.pushNumber(v)
        elseif t == "boolean" then 
            l.pushBoolean(v)
        else 
            l.pushNil() 
        end
    end

    local ok, err = pcall(function()
        l.pushString(var_name)
        _push_val(var_value)
        l.setGlobal(var_name)
    end)
    l.close()
    if not ok then
        error(string.format("setglobal error: %s", err))
    end
    return true
end

function M.getglobal(var_name)
    local ret = nil
    local l = luajava.new(L)
    local p = l.getPointer()
    if p == 0 or l.isClosed() then
        l.close()
        error("invalid or closed Lua state")
    end

    local ok, err = pcall(function()
        if l.getGlobal(var_name) ~= 0 then
            ret = _stack_to_lua(l, -1)
        end
    end)
    l.pop(l.getTop())
    l.close()
    if not ok then
        error(string.format("getglobal error: %s", err))
    end
    return ret
end

return M
