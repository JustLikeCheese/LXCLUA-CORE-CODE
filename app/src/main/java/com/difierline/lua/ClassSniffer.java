package com.difierline.lua;

import android.app.Activity;
import dalvik.system.BaseDexClassLoader;
import dalvik.system.PathClassLoader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;

class SimpleFilterBanBootImpl implements ClassFilter {
    @Override
    public boolean ban(String className) {
        if (className.contains("$$") ||
                className.contains("$1") || className.contains("$2") ||
                className.contains("$3") || className.contains("$4") ||
                className.contains("$5") || className.contains("$6") ||
                className.contains("$7") || className.contains("$8") ||
                className.contains("$9") || className.contains("$0") ||
                className.contains("$Lambda") || className.contains(".impl.") ||
                className.contains("Impl") || !className.contains(".")
        ) {
            return true;
        }
        byte[] bytes = className.getBytes();
        if (bytes.length <= 2) return true;
        int secondChar = bytes[1];
        if (secondChar == 46 || secondChar == 36) return true;
        if (bytes[bytes.length - 2] == 46 || bytes[bytes.length - 2] == 36) return true;
        if (ClassSniffer.checkPackageNaming(className)) return true;

        String[] prefixList = {
            "android.", "androidx.", "org.", "java.",
            "javax.", "com.android.", "com.framework.",
            "com.sun.", "libcore.", "dalvik.", "jdk.", "sun.","com.github.","com.difierline.","github."
        };
        for (String prefix : prefixList) {
            if (className.startsWith(prefix)) return false;
        }
        return true;
    }
}

class SimpleFilterBanApkImpl implements ClassFilter {
    @Override
    public boolean ban(String className) {
        if (className.contains("$$") ||
                className.contains("$1") || className.contains("$2") ||
                className.contains("$3") || className.contains("$4") ||
                className.contains("$5") || className.contains("$6") ||
                className.contains("$7") || className.contains("$8") ||
                className.contains("$9") || className.contains("$0") ||
                className.contains("$Lambda") || className.contains(".impl.") ||
                className.contains("Impl") || className.contains("._") ||
                className.contains(".-") || className.startsWith("_") ||
                className.startsWith("-")
        ) {
            return true;
        }
        byte[] bytes = className.getBytes();
        if (bytes.length <= 3) return true;
        int secondChar = bytes[1];
        if (secondChar == 46 || secondChar == 36) return true;
        int thirdChar = bytes[2];
        if (thirdChar == 46 || thirdChar == 36) return true;
        if (bytes[bytes.length - 2] == 46 || bytes[bytes.length - 2] == 36) return true;
        if (bytes[bytes.length - 3] == 46 || bytes[bytes.length - 3] == 36) return true;
        if (ClassSniffer.checkPackageNaming(className)) return true;
        return false;
    }
}

public class ClassSniffer {
    private final Activity activity;
    private final boolean banOnComplete;
    private boolean executed = false;
    private final HashMap<String, Boolean> bootClassCache = new HashMap<>();
    private final HashMap<String, Boolean> apkClassCache = new HashMap<>();
    private final ArrayList<String> processedFiles = new ArrayList<>();

    public static final ClassFilter SimpleFilterBanBoot = new SimpleFilterBanBootImpl();
    public static final ClassFilter SimpleFilterBanApk = new SimpleFilterBanApkImpl();

    private static final HashMap<String, Boolean> bootClassCacheStatic = new HashMap<>();
    private static final HashMap<String, Boolean> apkClassCacheStatic = new HashMap<>();
    private static final ArrayList<String> processedFilesStatic = new ArrayList<>();

    public static void addDexJustWithFile(File file, boolean toApkCache) {
        if (file.isFile() && file.canRead()) {
            try {
                ExtractDexs extractDexs = new ExtractDexs(file);
                ArrayList<byte[]> dexFiles = extractDexs.extract();
                for (byte[] dexBytes : dexFiles) {
                    DexFile dexFile = DexParser.fromBytes(dexBytes).parse();
                    String[] classNames = dexFile.getClassNames();
                    HashMap<String, Boolean> targetCache = toApkCache ? apkClassCacheStatic : bootClassCacheStatic;
                    for (String className : classNames) {
                        targetCache.put(className, true);
                    }
                }
            } catch (Throwable ignored) {
            }
        }
    }

    public static boolean checkPackageNaming(String str) {
        int dotIndex = str.indexOf('.');
        if (dotIndex == -1 || dotIndex == str.length() - 1) {
            return false;
        }
        int currentIndex = dotIndex;
        while (true) {
            int nextIndex = str.indexOf('.', currentIndex + 1);
            if (nextIndex == -1) {
                return false;
            }
            if (nextIndex - currentIndex == 2) {
                return true;
            }
            currentIndex = nextIndex;
        }
    }

    public static HashMap<String, Boolean> getApkDexCaches() {
        return apkClassCacheStatic;
    }

    public static HashMap<String, Boolean> getCaches() {
        return bootClassCacheStatic;
    }

    public ClassSniffer(Activity activity) {
        this(activity, true);
    }

    public ClassSniffer(Activity activity, boolean banOnComplete) {
        this.activity = activity;
        this.banOnComplete = banOnComplete;
    }

    public void banAll(ClassFilter filter) {
        banBoot(filter);
        banApk(filter);
    }

    public void banApk(ClassFilter filter) {
        execute();
        for (String key : apkClassCache.keySet()) {
            if (filter.ban(key)) {
                apkClassCache.put(key, false);
            }
        }
    }

    public void banBoot(ClassFilter filter) {
        execute();
        for (String key : bootClassCache.keySet()) {
            if (filter.ban(key)) {
                bootClassCache.put(key, false);
            }
        }
    }

    public boolean classExists(String className) {
        if (classExistsNotry(className)) {
            return true;
        }
        execute();
        return classExistsNotry(className);
    }

    private boolean classExistsNotry(String className) {
        Boolean exists = bootClassCache.get(className);
        return exists != null && exists;
    }

    public void execute() {
        if (executed) return;

        String packageResourcePath = activity.getPackageResourcePath();
        processDexFile(packageResourcePath, apkClassCache, processedFiles);

        ClassLoader classLoader = activity.getClassLoader();
        if (classLoader instanceof PathClassLoader) {
            try {
                PathClassLoader pathClassLoader = (PathClassLoader) classLoader;
                java.lang.reflect.Field[] baseDexFields = BaseDexClassLoader.class.getDeclaredFields();
                for (java.lang.reflect.Field baseDexField : baseDexFields) {
                    baseDexField.setAccessible(true);
                    String fieldType = baseDexField.getType().getName();
                    String fieldName = baseDexField.getName();
                    if (fieldType.equals("dalvik.system.DexPathList") || fieldName.equals("pathList")) {
                        Object dexPathList = baseDexField.get(pathClassLoader);
                        java.lang.reflect.Field[] pathListFields = dexPathList.getClass().getDeclaredFields();
                        for (java.lang.reflect.Field pathListField : pathListFields) {
                            pathListField.setAccessible(true);
                            String pathListFieldType = pathListField.getType().getName();
                            String pathListFieldName = pathListField.getName();
                            if (pathListFieldType.equals("[Ldalvik.system.DexPathList$Element;") || pathListFieldName.equals("dexElements")) {
                                Object[] dexElements = (Object[]) pathListField.get(dexPathList);
                                if (dexElements != null) {
                                    for (Object element : dexElements) {
                                        String elementStr = element.toString();
                                        int startIndex = elementStr.indexOf('"') + 1;
                                        int endIndex = elementStr.lastIndexOf('"');
                                        if (startIndex >= 1 && endIndex > startIndex) {
                                            String dexPath = elementStr.substring(startIndex, endIndex);
                                            if (!dexPath.equals(packageResourcePath) && !processedFiles.contains(dexPath)) {
                                                processDexFile(dexPath, apkClassCache, processedFiles);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } catch (Exception ignored) {
            }
        }

        try {
            InputStream inputStream = activity.getAssets().open("android-strip.dex");
            ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
            byte[] buffer = new byte[1024];
            int read;
            while ((read = inputStream.read(buffer)) != -1) {
                byteArrayOutputStream.write(buffer, 0, read);
            }
            inputStream.close();
            DexFile dexFile = DexParser.fromBytes(byteArrayOutputStream.toByteArray()).parse();
            for (String className : dexFile.getClassNames()) {
                apkClassCache.put(className, true);
            }
        } catch (Throwable ignored) {
        }

        executed = true;
        if (banOnComplete) {
            try {
                banBoot(SimpleFilterBanBoot);
                banApk(SimpleFilterBanApk);
            } catch (Exception ignored) {
            }
        }
    }

    private void processDexFile(String filePath, HashMap<String, Boolean> cache, ArrayList<String> processedList) {
        if (processedList.contains(filePath)) return;

        File file = new File(filePath);
        if (file.isFile() && file.canRead()) {
            try {
                ExtractDexs extractDexs = new ExtractDexs(filePath);
                ArrayList<byte[]> dexFiles = extractDexs.extract();
                for (byte[] dexBytes : dexFiles) {
                    DexFile dexFile = DexParser.fromBytes(dexBytes).parse();
                    for (String className : dexFile.getClassNames()) {
                        cache.put(className, true);
                    }
                }
            } catch (Throwable ignored) {
            }
        }
        processedList.add(filePath);
    }

    public ArrayList<String> getClasses(boolean sorted) {
        execute();
        ArrayList<String> result = new ArrayList<>();
        for (String key : bootClassCache.keySet()) {
            if (bootClassCache.get(key)) {
                result.add(key);
            }
        }
        for (String key : apkClassCache.keySet()) {
            if (apkClassCache.get(key)) {
                result.add(key);
            }
        }
        if (sorted) {
            Collections.sort(result);
        }
        return result;
    }

    public ClassResult getClassesResult(boolean sorted) {
        execute();
        ArrayList<String> result = new ArrayList<>();
        for (String key : bootClassCache.keySet()) {
            if (bootClassCache.get(key)) {
                result.add(key);
            }
        }
        for (String key : apkClassCache.keySet()) {
            if (apkClassCache.get(key)) {
                result.add(key);
            }
        }
        if (sorted) {
            Collections.sort(result);
        }
        return new ClassResult(result);
    }
}