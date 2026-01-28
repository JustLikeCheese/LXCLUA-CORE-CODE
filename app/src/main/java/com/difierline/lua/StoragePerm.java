package com.difierline.lua;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;
import android.util.Log;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;

public class StoragePerm {

    private static final String PREFS = "storagePerm";
    private static final String SKIP = "skip";

    public static boolean request(Activity activity, boolean isForced) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) return true;
        if (check(activity)) return true;
        MaterialAlertDialogBuilder builder = new MaterialAlertDialogBuilder(activity)
            .setTitle("权限申请")
            .setCancelable(false);
        if (isForced) {
            builder.setMessage("必须授予权限后才能使用本应用！\n\n为了正常使用应用的所有功能，需要获取\"所有文件访问权限\"。\n\n此权限用于访问和管理应用的文件数据，包括脚本文件、配置文件等。\n\n请授予权限后重启本应用。")
                .setPositiveButton("前往开启", (dialog, which) -> {
                    open(activity);
                    activity.finishAffinity();
                });
        } else {
            builder.setMessage("为了正常使用应用的所有功能，需要获取\"所有文件访问权限\"。\n\n此权限用于访问和管理应用的文件数据，包括脚本文件、配置文件等。\n\n是否前往开启权限？")
                .setPositiveButton("前往开启", (dialog, which) -> open(activity))
                .setNegativeButton("暂不需要", (dialog, which) -> {
                    dialog.dismiss();
                    activity.getSharedPreferences(PREFS, Activity.MODE_PRIVATE).edit().putBoolean(SKIP, true).apply();
                });
        }
        builder.show();
        return false;
    }
    
    public static boolean request(Activity activity) {
        return request(activity, false);
    }

    private static void open(Activity activity) {
        try {
            Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
            intent.setData(Uri.parse("package:" + activity.getPackageName()));
            activity.startActivity(intent);
        } catch (Exception e) {
            Log.e("StoragePerm", "跳转失败: " + e.getMessage());
            try {
                Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
                intent.setData(Uri.parse("package:" + activity.getPackageName()));
                activity.startActivity(intent);
            } catch (Exception e2) {
                Log.e("StoragePerm", "跳转失败: " + e2.getMessage());
            }
        }
    }

    public static void reset(Activity activity) {
        activity.getSharedPreferences(PREFS, Activity.MODE_PRIVATE).edit().putBoolean(SKIP, false).apply();
    }

    private static boolean check(Activity activity) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (android.os.Environment.isExternalStorageManager()) return true;
        }
        return activity.getSharedPreferences(PREFS, Activity.MODE_PRIVATE).getBoolean(SKIP, false);
    }
}
