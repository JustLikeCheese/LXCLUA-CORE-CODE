package com.difierline.lua;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.os.Build;
import android.os.Environment;

import com.luajava.LuaException;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;

import dalvik.system.DexClassLoader;

public class LuaDexLoader {
    private static HashMap<String, LuaDexClassLoader> dexCache = new HashMap<String, LuaDexClassLoader>();
    private ArrayList<ClassLoader> dexList = new ArrayList<ClassLoader>();
    private HashMap<String, String> libCache = new HashMap<String, String>();

    private LuaContext mContext;

    private String luaDir;

    private AssetManager mAssetManager;

    private LuaResources mResources;
    private Resources.Theme mTheme;
    private String odexDir;
    private String privateLibsDir;

    public LuaDexLoader(LuaContext context) {
        mContext = context;
        luaDir = context.getLuaDir();
        LuaApplication app = LuaApplication.getInstance();
        //localDir = app.getLocalDir();
        odexDir = app.getOdexDir();
        
        // 初始化私有libs目录
        Context ctx = mContext.getContext();
        privateLibsDir = new File(ctx.getFilesDir(), "Link").getAbsolutePath();
        File dir = new File(privateLibsDir);
        if (!dir.exists()) {
            dir.mkdirs();
        }
    }

    public Resources.Theme getTheme() {
        return mTheme;
    }

    public ArrayList<ClassLoader> getClassLoaders() {
        // TODO: Implement this method
        return dexList;
    }

    public LuaDexClassLoader loadApp(String pkg) {
        try {
            LuaDexClassLoader dex = dexCache.get(pkg);
            if (dex == null) {
                PackageManager manager = mContext.getContext().getPackageManager();
                ApplicationInfo info = manager.getPackageInfo(pkg, 0).applicationInfo;
                dex = new LuaDexClassLoader(info.publicSourceDir, LuaApplication.getInstance().getOdexDir(), info.nativeLibraryDir, mContext.getContext().getClassLoader());
                dexCache.put(pkg, dex);
            }
            if (!dexList.contains(dex)) {
                dexList.add(dex);
            }
            return dex;
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        return null;
    }


    public void loadLibs() throws LuaException {
        File[] libs = new File(mContext.getLuaDir() + "/libs").listFiles();
        if (libs == null)
            return;
        for (File f : libs) {
            if(f.isDirectory())
                continue;
            if (f.getAbsolutePath().endsWith(".so"))
                loadLib(f.getName());
            else
                loadDex(f.getAbsolutePath());
        }
    }

    public void loadLib(String name) throws LuaException {
        String fn = name;
        int i = name.indexOf(".");
        if (i > 0)
            fn = name.substring(0, i);
        if (fn.startsWith("lib"))
            fn = fn.substring(3);
        String libDir = mContext.getContext().getDir(fn, Context.MODE_PRIVATE).getAbsolutePath();
        String libPath = libDir + "/lib" + fn + ".so";
        File f = new File(libPath);
        if (!f.exists()) {
            f = new File(luaDir + "/libs/lib" + fn + ".so");
            if (!f.exists())
                throw new LuaException("找不到库: " + name);
            LuaUtil.copyFile(luaDir + "/libs/lib" + fn + ".so", libPath);

        }
        libCache.put(fn, libPath);
    }

    public HashMap<String, String> getLibrarys() {
        return libCache;
    }


    public DexClassLoader loadDex(String path) throws LuaException {
        LuaDexClassLoader dex = dexCache.get(path);
        if(dex==null)
           dex = loadApp(path);
        if (dex == null){
            String name = path;
            if (path.charAt(0) != '/')
                path = luaDir + "/" + path;
            
            // 检查文件是否存在
            File srcFile = new File(path);
            if (!srcFile.exists()) {
                if (new File(path + ".dex").exists())
                    path += ".dex";
                else if (new File(path + ".jar").exists())
                    path += ".jar";
                else
                    throw new LuaException(path + " not found");
                srcFile = new File(path);
            }
            
            // Android 14+ 需要特殊处理
            String finalPath = path;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) { // API 34
                // Android 14+：文件必须是只读的
                finalPath = prepareFileForAndroid14(srcFile);
            }
            
            String id = LuaUtil.getFileMD5(finalPath);
            if (id != null && id.equals("0"))
                id = name;
            dex = dexCache.get(id);

            if (dex == null) {
                dex = new LuaDexClassLoader(finalPath, odexDir, 
                        LuaApplication.getInstance().getApplicationInfo().nativeLibraryDir, 
                        mContext.getContext().getClassLoader());
                dexCache.put(id, dex);
            }
        }

        if (!dexList.contains(dex)) {
            dexList.add(dex);
            String dexPath = dex.getDexPath();
            if (dexPath.endsWith(".jar"))
                loadResources(dexPath);
        }
        return dex;
    }

    public void loadResources(String path) {
        try {
            AssetManager assetManager = AssetManager.class.newInstance();
            Method addAssetPath = assetManager.getClass().getMethod("addAssetPath", String.class);
            int ok = (int) addAssetPath.invoke(assetManager, path);
            if (ok == 0)
                return;
            mAssetManager = assetManager;
            Resources superRes = mContext.getContext().getResources();
            mResources = new LuaResources(mAssetManager, superRes.getDisplayMetrics(),
                    superRes.getConfiguration());
            mResources.setSuperResources(superRes);
            mTheme = mResources.newTheme();
            mTheme.setTo(mContext.getContext().getTheme());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public AssetManager getAssets() {
        return mAssetManager;
    }

    public Resources getResources() {
        return mResources;
    }
    
    /**
     * Android 14+ 特殊处理：确保文件是只读的
     */
    private String prepareFileForAndroid14(File srcFile) throws LuaException {
        try {
            // 检查文件是否在外部存储中
            if (isInExternalStorage(srcFile)) {
                // 复制到私有目录
                return copyToPrivateDirWithReadOnly(srcFile);
            } else {
                // 文件已经在私有目录，确保是只读的
                if (srcFile.exists() && srcFile.canWrite()) {
                    srcFile.setReadOnly();
                }
                return srcFile.getAbsolutePath();
            }
        } catch (Exception e) {
            throw new LuaException("Failed to prepare file for Android 14: " + e.getMessage());
        }
    }

    /**
     * 检查文件是否在外部存储（可写的公共目录）中
     */
    private boolean isInExternalStorage(File file) {
        try {
            if (Environment.isExternalStorageRemovable()) {
                return false;
            }
            
            String filePath = file.getAbsolutePath();
            String[] externalPaths = {
                Environment.getExternalStorageDirectory().getAbsolutePath(),
                "/sdcard",
                "/storage/emulated",
                "/mnt/sdcard"
            };
            
            for (String externalPath : externalPaths) {
                if (filePath.startsWith(externalPath)) {
                    return true;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    /**
     * 将文件复制到应用私有目录并设置为只读（Android 14+ 安全模式）
     */
    private String copyToPrivateDirWithReadOnly(File srcFile) throws LuaException, IOException {
        // 创建私有目录（如果不存在）
        File dir = new File(privateLibsDir);
        if (!dir.exists()) {
            if (!dir.mkdirs()) {
                throw new LuaException("Failed to create directory: " + privateLibsDir);
            }
        }
        
        // 创建目标文件路径（保持文件名）
        String fileName = srcFile.getName();
        File destFile = new File(dir, fileName);
        
        // 如果文件已存在，检查是否需要更新
        if (destFile.exists()) {
            // 检查文件大小和MD5是否相同
            long srcSize = srcFile.length();
            long destSize = destFile.length();
            
            if (srcSize == destSize) {
                // 文件大小相同，检查MD5
                String srcMd5 = LuaUtil.getFileMD5(srcFile.getAbsolutePath());
                String destMd5 = LuaUtil.getFileMD5(destFile.getAbsolutePath());
                
                if (srcMd5 != null && destMd5 != null && srcMd5.equals(destMd5)) {
                    // 文件相同，确保文件是只读的
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
                        destFile.setReadOnly();
                    }
                    return destFile.getAbsolutePath();
                }
            }
            
            // MD5不同，比较文件新旧
            long srcLastModified = srcFile.lastModified();
            long destLastModified = destFile.lastModified();
            
            if (srcLastModified > destLastModified) {
                // 源文件更新，用新文件替换旧文件
                if (!destFile.delete()) {
                    // 如果删除失败，重命名为.old文件
                    File oldFile = new File(dir, fileName + ".old." + destLastModified);
                    destFile.renameTo(oldFile);
                }
            } else {
                // 目标文件更新或相同，无需复制，直接返回
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
                    destFile.setReadOnly();
                }
                return destFile.getAbsolutePath();
            }
        }
        
        // 根据文档要求，在写入前将文件设置为只读（避免竞态条件）
        // 先创建空文件并设置为只读
        destFile.createNewFile();
        destFile.setReadOnly();
        
        // 但是，我们需要写入文件内容，所以需要重新打开为可写
        // 创建一个临时文件写入，然后重命名
        File tempFile = new File(dir, fileName + ".tmp." + System.currentTimeMillis());
        
        try {
            // 复制文件到临时文件
            LuaUtil.copyFile(srcFile.getAbsolutePath(), tempFile.getAbsolutePath());
            
            // 验证复制是否成功
            if (!tempFile.exists()) {
                throw new LuaException("Failed to create temporary file");
            }
            
            long srcSize = srcFile.length();
            long tempSize = tempFile.length();
            if (srcSize != tempSize) {
                throw new LuaException("File size mismatch (source: " + srcSize + ", temp: " + tempSize + ")");
            }
            
            // 删除只读的空文件
            destFile.delete();
            
            // 将临时文件重命名为目标文件
            if (!tempFile.renameTo(destFile)) {
                throw new LuaException("Failed to rename temporary file to destination");
            }
            
            // 设置目标文件为只读
            destFile.setReadOnly();
            
            // 验证最终文件
            if (!destFile.exists()) {
                throw new LuaException("Destination file does not exist after rename");
            }
            
            return destFile.getAbsolutePath();
            
        } finally {
            // 清理临时文件
            if (tempFile.exists()) {
                tempFile.delete();
            }
        }
    }

    /**
     * 清理私有目录中的旧文件
     */
    public void cleanupOldFiles() {
        try {
            File dir = new File(privateLibsDir);
            if (!dir.exists() || !dir.isDirectory()) {
                return;
            }
            
            File[] files = dir.listFiles();
            if (files == null) {
                return;
            }
            
            // 检查是否需要清理（每天最多一次）
            android.content.SharedPreferences prefs = mContext.getContext().getSharedPreferences("dex_loader", android.content.Context.MODE_PRIVATE);
            long lastCleanupTime = prefs.getLong("last_cleanup_time", 0);
            long now = System.currentTimeMillis();
            if (now - lastCleanupTime < 24 * 60 * 60 * 1000) {
                return;
            }
            
            // 更新上次清理时间
            prefs.edit().putLong("last_cleanup_time", now).apply();
            
            // 分类文件
            java.util.List<File> tempFiles = new java.util.ArrayList<>();
            java.util.List<File> oldFiles = new java.util.ArrayList<>();
            java.util.List<File> normalFiles = new java.util.ArrayList<>();
            long cutoff = now - (7L * 24 * 60 * 60 * 1000); // 7天
            
            for (File file : files) {
                if (file.getName().contains(".tmp.")) {
                    tempFiles.add(file);
                } else if (file.getName().contains(".old.")) {
                    oldFiles.add(file);
                } else if (file.lastModified() < cutoff) {
                    normalFiles.add(file);
                }
            }
            
            // 清理临时文件（优先清理，最容易清理）
            int cleaned = 0;
            for (File file : tempFiles) {
                if (file.delete()) {
                    cleaned++;
                }
            }
            
            // 清理.old文件
            for (File file : oldFiles) {
                if (file.delete()) {
                    cleaned++;
                }
            }
            
            // 如果正常文件太多（超过100个），清理最旧的
            if (normalFiles.size() > 100) {
                // 按修改时间排序
                normalFiles.sort((a, b) -> Long.compare(a.lastModified(), b.lastModified()));
                int toRemove = normalFiles.size() - 100;
                for (int i = 0; i < toRemove && i < normalFiles.size(); i++) {
                    if (normalFiles.get(i).delete()) {
                        cleaned++;
                    }
                }
            }
            
            // 清理过旧的正常文件（超过30天）
            for (File file : normalFiles) {
                if (file.lastModified() < now - (30L * 24 * 60 * 60 * 1000)) {
                    if (file.delete()) {
                        cleaned++;
                    }
                }
            }
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
