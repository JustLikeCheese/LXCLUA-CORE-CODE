package com.difierline.lua;

import android.app.Application;
import android.content.Context;
import androidx.core.content.FileProvider;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.widget.Toast;

import com.luajava.LuaState;
import com.luajava.LuaTable;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import com.difierline.lua.lxclua.CrashManager;

public class LuaApplication extends Application implements LuaContext {

    private static LuaApplication mApp;
    static private HashMap<String, Object> data = new HashMap<String, Object>();
    protected String localDir;
    protected String odexDir;
    protected String libDir;
    protected String luaMdDir;
    protected String luaCpath;
    protected String luaLpath = ""; // 初始化为空字符串
    protected String luaExtDir;
    private boolean isUpdata;
    private SharedPreferences mSharedPreferences;

    public Uri getUriForPath(String path) {
        return FileProvider.getUriForFile(this, getPackageName(), new File(path));
    }

    public Uri getUriForFile(File path) {
        return FileProvider.getUriForFile(this, getPackageName(), path);
    }

    public String getPathFromUri(Uri uri) {
        String path = null;
        if (uri != null) {
            switch (uri.getScheme()) {
                case "content":
                    Cursor cursor = getContentResolver().query(uri, null, null, null, null);
                    if (cursor != null && cursor.moveToFirst()) {
                        int idx = cursor.getColumnIndex("_data");
                        if (idx >= 0) {
                            path = cursor.getString(idx);
                        }
                        cursor.close();
                    }
                    break;
                case "file":
                    path = uri.getPath();
                    break;
            }
        }
        return path;
    }

    public static LuaApplication getInstance() {
        return mApp;
    }

    @Override
    public ArrayList<ClassLoader> getClassLoaders() {
        return new ArrayList<>();
    }

    @Override
    public void regGc(LuaGcable obj) {
        // 待实现
    }

    @Override
    public String getLuaPath() {
        return localDir;
    }

    @Override
    public String getLuaPath(String path) {
        return new File(getLuaDir(), path).getAbsolutePath();
    }

    @Override
    public String getLuaPath(String dir, String name) {
        return new File(getLuaDir(dir), name).getAbsolutePath();
    }

    @Override
    public String getLuaExtPath(String path) {
        return new File(getLuaExtDir(), path).getAbsolutePath();
    }

    @Override
    public String getLuaExtPath(String dir, String name) {
        return new File(getLuaExtDir(dir), name).getAbsolutePath();
    }

    public int getWidth() {
        return getResources().getDisplayMetrics().widthPixels;
    }

    public int getHeight() {
        return getResources().getDisplayMetrics().heightPixels;
    }

    @Override
    public String getLuaDir(String dir) {
        return new File(localDir, dir).getAbsolutePath();
    }

    @Override
    public String getLuaExtDir(String name) {
        File dir = new File(luaExtDir, name);
        if (!dir.exists() && !dir.mkdirs()) {
            return dir.getAbsolutePath();
        }
        return dir.getAbsolutePath();
    }

    public String getLibDir() {
        return libDir;
    }

    public String getOdexDir() {
        return odexDir;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        CrashManager.install(this);
        
        mApp = this;
        mSharedPreferences = getSharedPreferences(this);
        
        // 初始化XCLUA工作目录
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            String sdDir = Environment.getExternalStorageDirectory().getAbsolutePath();
            luaExtDir = sdDir + "/XCLUA";
        } else {
            File storageDir = new File("/storage");
            File[] fs = storageDir.listFiles();
            if (fs != null) {
                for (File f : fs) {
                    if (f.list() != null && f.list().length > 5) {
                        luaExtDir = f.getAbsolutePath() + "/XCLUA";
                        break;
                    }
                }
            }
            if (luaExtDir == null) {
                luaExtDir = getDir("XCLUA", Context.MODE_PRIVATE).getAbsolutePath();
            }
        }

        File destDir = new File(luaExtDir);
        if (!destDir.exists() && !destDir.mkdirs()) {
            // 处理目录创建失败的情况
            luaExtDir = getFilesDir().getAbsolutePath() + "/XCLUA";
            destDir = new File(luaExtDir);
            destDir.mkdirs();
        }

        // 定义文件夹
        localDir = getFilesDir().getAbsolutePath();
        odexDir = getDir("odex", Context.MODE_PRIVATE).getAbsolutePath();
        libDir = getDir("lib", Context.MODE_PRIVATE).getAbsolutePath();
        luaMdDir = getDir("lua", Context.MODE_PRIVATE).getAbsolutePath();
        luaCpath = getApplicationInfo().nativeLibraryDir + "/lib?.so" + ";" + libDir + "/lib?.so";
        
        // 正确初始化 luaLpath
        File manifestFile = new File(luaMdDir, "manifest.json");
        if (!manifestFile.exists()) {
            luaLpath = luaMdDir + "/?.lua;" + luaMdDir + "/?.luac;" + luaMdDir + "/lua/?.lua;" + luaMdDir + "/lua/?.luac;" + luaMdDir + "/lua/?.png;" + luaMdDir + "/?/manifest.json;" + luaMdDir + "/?.png;";
        } else {
            luaLpath += luaMdDir + "/?.lua;" + luaMdDir + "/?.luac;" + luaMdDir + "/lua/?.lua;" + luaMdDir + "/lua/?.luac;" + luaMdDir + "/lua/?.png;" + luaMdDir + "/?/init.lua;" + luaMdDir + "/?.png;";
        }
    }

    private static SharedPreferences getSharedPreferences(Context context) {
        return PreferenceManager.getDefaultSharedPreferences(context);
    }

    @Override
    public String getLuaDir() {
        return localDir;
    }

    @Override
    public void call(String name, Object[] args) {
        // 待实现
    }

    @Override
    public void set(String name, Object object) {
        data.put(name, object);
    }

    @Override
    public Map<String, Object> getGlobalData() {
        return data;
    }
    
    public void clearSharedData() {
    SharedPreferences.Editor editor = mSharedPreferences.edit();
    editor.clear();  // 清除所有键值对
    editor.apply();  // 异步提交更改
}

    @Override
    public Map<String, ?> getSharedData() {
        return mSharedPreferences.getAll();
    }

    @Override
    public Object getSharedData(String key) {
        return mSharedPreferences.getAll().get(key);
    }

    @Override
    public Object getSharedData(String key, Object def) {
        Object ret = mSharedPreferences.getAll().get(key);
        return ret != null ? ret : def;
    }

    @Override
    @SuppressWarnings("unchecked")
    public boolean setSharedData(String key, Object value) {
        SharedPreferences.Editor edit = mSharedPreferences.edit();
        if (value == null) {
            edit.remove(key);
        } else if (value instanceof String) {
            edit.putString(key, (String) value);
        } else if (value instanceof Long) {
            edit.putLong(key, (Long) value);
        } else if (value instanceof Integer) {
            edit.putInt(key, (Integer) value);
        } else if (value instanceof Float) {
            edit.putFloat(key, (Float) value);
        } else if (value instanceof Set) {
            edit.putStringSet(key, (Set<String>) value);
        } else if (value instanceof LuaTable) {
            // 安全地将LuaTable转换为Set<String>
            LuaTable table = (LuaTable) value;
            Set<String> set = new HashSet<>();
            for (int i = 1; i <= table.length(); i++) {
                Object val = table.get(i);
                if (val instanceof String) {
                    set.add((String) val);
                }
            }
            edit.putStringSet(key, set);
        } else if (value instanceof Boolean) {
            edit.putBoolean(key, (Boolean) value);
        } else {
            return false;
        }
        edit.apply();
        return true;
    }

    public Object get(String name) {
        return data.get(name);
    }

    public String getLocalDir() {
        return localDir;
    }

    public String getMdDir() {
        return luaMdDir;
    }

    @Override
    public String getLuaExtDir() {
        return luaExtDir;
    }

    @Override
    public void setLuaExtDir(String dir) {
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            String sdDir = Environment.getExternalStorageDirectory().getAbsolutePath();
            luaExtDir = new File(sdDir, dir).getAbsolutePath();
        } else {
            File storageDir = new File("/storage");
            File[] fs = storageDir.listFiles();
            if (fs != null) {
                for (File f : fs) {
                    String[] ls = f.list();
                    if (ls != null && ls.length > 5) {
                        luaExtDir = new File(f, dir).getAbsolutePath();
                        break;
                    }
                }
            }
            if (luaExtDir == null) {
                luaExtDir = getDir(dir, Context.MODE_PRIVATE).getAbsolutePath();
            }
        }
        
        // 确保目录存在
        File destDir = new File(luaExtDir);
        if (!destDir.exists()) {
            destDir.mkdirs();
        }
    }

    @Override
    public String getLuaLpath() {
        return luaLpath;
    }

    @Override
    public String getLuaCpath() {
        return luaCpath;
    }

    @Override
    public Context getContext() {
        return this;
    }

    @Override
    public LuaState getLuaState() {
        return null;
    }

    @Override
    public Object doFile(String path, Object[] arg) {
        return null;
    }

    @Override
    public void sendMsg(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
    }

    @Override
    public void sendError(String title, Exception msg) {
        // 待实现
    }
}