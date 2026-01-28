local BitmapUtil = luajava.bindClass "com.difierline.lua.lxclua.utils.BitmapUtil"

function loadbitmap(path)
  if path:find("^https*://") then
    return BitmapUtil.fromUrl(path)
   elseif not path:find("^/") then
    return BitmapUtil.fromFile(string.format("%s/%s", luajava.luadir, path))
   else
    return BitmapUtil.fromFile(path)
  end
end