package com.difierline.lua;

import android.content.Intent;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.util.Log;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.Thread;
import java.lang.reflect.Field;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.LinkedHashMap;
import java.util.Map;

public class CrashHandler implements Thread.UncaughtExceptionHandler {
    private static CrashHandler INSTANCE = new CrashHandler();
    public static final String TAG = "CrashHandler";
    private Context mContext;
    private Thread.UncaughtExceptionHandler mDefaultHandler;
    private Map<String, String> infos = new LinkedHashMap();
    private DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd-HH-mm-ss");

    CrashHandler() {
    }

    public static CrashHandler getInstance() {
        return INSTANCE;
    }

    public void init(Context context) {
        this.mContext = context;
        this.mDefaultHandler = Thread.getDefaultUncaughtExceptionHandler();
        Thread.setDefaultUncaughtExceptionHandler(this);
    }

    @Override
    public void uncaughtException(Thread thread, Throwable th) {
        if (handleException(th) || this.mDefaultHandler == null) {
            return;
        }
        this.mDefaultHandler.uncaughtException(thread, th);
    }

    private boolean handleException(Throwable th) {
        if (th == null) {
            return false;
        }
        collectDeviceInfo(this.mContext);
        saveCrashInfo2File(th);
        return true;
    }

    public void collectDeviceInfo(Context context) {
        Field[] declaredFields;
        Field[] declaredFields2;
        try {
            PackageInfo packageInfo = context.getPackageManager().getPackageInfo(context.getPackageName(), 1);
            if (packageInfo != null) {
                String str = packageInfo.versionName == null ? "null" : packageInfo.versionName;
                String stringBuffer = new StringBuffer().append(packageInfo.versionCode).append("").toString();
                this.infos.put("versionName", str);
                this.infos.put("versionCode", stringBuffer);
            }
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(TAG, "an error occured when collect package info", e);
        }
        try {
            for (Field field : Class.forName("android.os.Build").getDeclaredFields()) {
                try {
                    field.setAccessible(true);
                    Object obj = field.get(null);
                    if (obj instanceof String[]) {
                        this.infos.put(field.getName(), Arrays.toString((String[]) obj));
                    } else {
                        this.infos.put(field.getName(), obj.toString());
                    }
                    Log.d(TAG, new StringBuffer().append(new StringBuffer().append(field.getName()).append(" : ").toString()).append(field.get(null)).toString());
                } catch (Exception e2) {
                    Log.e(TAG, "收集崩溃信息时发生错误", e2);
                }
            }
            try {
                for (Field field2 : Class.forName("android.os.Build$VERSION").getDeclaredFields()) {
                    try {
                        field2.setAccessible(true);
                        Object obj2 = field2.get(null);
                        if (obj2 instanceof String[]) {
                            this.infos.put(field2.getName(), Arrays.toString((String[]) obj2));
                        } else {
                            this.infos.put(field2.getName(), obj2.toString());
                        }
                        Log.d(TAG, new StringBuffer().append(new StringBuffer().append(field2.getName()).append(" : ").toString()).append(field2.get(null)).toString());
                    } catch (Exception e3) {
                        Log.e(TAG, "an error occured when collect crash info", e3);
                    }
                }
            } catch (ClassNotFoundException e4) {
                throw new NoClassDefFoundError(e4.getMessage());
            }
        } catch (ClassNotFoundException e5) {
            throw new NoClassDefFoundError(e5.getMessage());
        }
    }

    private String saveCrashInfo2File(Throwable th) {
        StringBuffer stringBuffer = new StringBuffer();
        for (Map.Entry<String, String> entry : this.infos.entrySet()) {
            String key = entry.getKey();
            stringBuffer.append(new StringBuffer().append(new StringBuffer().append(new StringBuffer().append(key).append("=").toString()).append(entry.getValue()).toString()).append("\n").toString());
        }
        StringWriter stringWriter = new StringWriter();
        PrintWriter printWriter = new PrintWriter(stringWriter);
        th.printStackTrace(printWriter);
        Throwable cause = th.getCause();
        while (true) {
            Throwable th2 = cause;
            if (th2 == null) {
                break;
            }
            th2.printStackTrace(printWriter);
            cause = th2.getCause();
        }
        printWriter.close();
        stringBuffer.append(stringWriter.toString());
        try {
            String stringBuffer2 = new StringBuffer().append(new StringBuffer().append(new StringBuffer().append(new StringBuffer().append("crash-").append(this.formatter.format(new Date())).toString()).append("-").toString()).append(System.currentTimeMillis()).toString()).append(".log").toString();
            if (Environment.getExternalStorageState().equals("mounted")) {
                
                
                File file = new File("/sdcard/XCLUA/crash/");
                if (!file.exists()) {
                    file.mkdirs();
                }
                FileOutputStream fileOutputStream = new FileOutputStream(new StringBuffer().append("/sdcard/XCLUA/crash/").append(stringBuffer2).toString());
                fileOutputStream.write(stringBuffer.toString().getBytes());
                Log.e("crash", stringBuffer.toString());
                fileOutputStream.close();
                
            }
            return stringBuffer2;
        } catch (Exception e) {
            Log.e(TAG, "写入文件时发生错误", e);
            return null;
        }
    }
}
