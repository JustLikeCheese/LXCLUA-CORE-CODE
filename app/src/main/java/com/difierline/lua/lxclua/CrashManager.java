package com.difierline.lua.lxclua;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Bundle;
import android.os.Build;
import android.util.Log;
import android.util.TypedValue;

import java.io.PrintWriter;
import java.io.Serializable;
import java.io.StringWriter;
import java.lang.ref.WeakReference;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Locale;

import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.style.ForegroundColorSpan;
import android.text.style.StyleSpan;
import android.graphics.Typeface;
import androidx.core.content.ContextCompat;

import com.difierline.lua.lxclua.R;

@SuppressLint("NewApi")
public final class CrashManager {

    private static final String EXTRA_RESTART_ACTIVITY_CLASS = "EXTRA_RESTART_ACTIVITY_CLASS";
    private static final String EXTRA_SHOW_ERROR_DETAILS = "EXTRA_SHOW_ERROR_DETAILS";
    private static final String EXTRA_STACK_TRACE = "EXTRA_STACK_TRACE";
    private static final String EXTRA_EXCEPTION_TYPE = "EXTRA_EXCEPTION_TYPE";
    private static final String EXTRA_THREAD_INFO = "EXTRA_THREAD_INFO";
    private static final String EXTRA_CRASH_CONTEXT = "EXTRA_CRASH_CONTEXT";

    private static final String TAG = "crashmanager";
    private static final String INTENT_ACTION_ERROR_ACTIVITY = "ERROR";
    private static final String INTENT_ACTION_RESTART_ACTIVITY = "RESTART";
    private static final String CAOC_HANDLER_PACKAGE_NAME = "com.crashmanager";
    private static final String DEFAULT_HANDLER_PACKAGE_NAME = "com.android.internal.os";
    private static final int MAX_STACK_TRACE_SIZE = 131071; // 128 KB - 1

    private static Application application;
    private static WeakReference<Activity> lastActivityCreated = new WeakReference<>(null);
    private static boolean isInBackground = false;

    private static boolean launchErrorActivityWhenInBackground = true;
    private static boolean showErrorDetails = true;
    private static boolean enableAppRestart = true;
    private static Class<? extends Activity> errorActivityClass = null;
    private static Class<? extends Activity> restartActivityClass = null;

    public static void install(Context context) {
        try {
            if (context == null) {
                Log.e(TAG, "安装失败：context为null！");
            } else {
                Thread.UncaughtExceptionHandler oldHandler =
                        Thread.getDefaultUncaughtExceptionHandler();

                if (oldHandler != null
                        && oldHandler.getClass().getName().startsWith(CAOC_HANDLER_PACKAGE_NAME)) {
                    Log.e(TAG, "崩溃管理器已安装，无需重复安装！");
                } else {
                    if (oldHandler != null
                            && !oldHandler
                                    .getClass()
                                    .getName()
                                    .startsWith(DEFAULT_HANDLER_PACKAGE_NAME)) {
                        Log.e(
                                TAG,
                                "重要警告！已存在UncaughtExceptionHandler，确定要这样做吗？如果使用ACRA、Crashlytics等库，必须在崩溃管理器之后初始化它们！仍将安装，但原始处理器将不会被调用。");
                    }

                    application = (Application) context.getApplicationContext();

                    Thread.setDefaultUncaughtExceptionHandler(
                            (thread, throwable) -> {
                                Log.e(
                                        TAG,
                                        "App has crashed, executing crashmanager's UncaughtExceptionHandler",
                                        throwable);

                                if (errorActivityClass == null) {
                                    errorActivityClass = guessErrorActivityClass(application);
                                }

                                if (isStackTraceLikelyConflictive(
                                        throwable, errorActivityClass)) {
                                    Log.e(
                                            TAG,
                                            "您的应用程序类或错误活动已崩溃，将不会启动自定义错误页面！");
                                } else {
                                    if (launchErrorActivityWhenInBackground
                                            || !isInBackground) {
                                        final Intent intent =
                                                new Intent(application, errorActivityClass);
                                        StringWriter sw = new StringWriter();
                                        PrintWriter pw = new PrintWriter(sw);
                                        throwable.printStackTrace(pw);
                                        String stackTraceString = sw.toString();

                                        if (stackTraceString.length() > MAX_STACK_TRACE_SIZE) {
                                            String disclaimer = " [堆栈跟踪过长已截断]";
                                            stackTraceString =
                                                    stackTraceString.substring(
                                                                    0,
                                                                    MAX_STACK_TRACE_SIZE
                                                                            - disclaimer
                                                                                    .length())
                                                            + disclaimer;
                                        }

                                        if (enableAppRestart && restartActivityClass == null) {
                                            restartActivityClass =
                                                    guessRestartActivityClass(application);
                                        } else if (!enableAppRestart) {
                                            restartActivityClass = null;
                                        }

                                        intent.putExtra(EXTRA_STACK_TRACE, stackTraceString);
                                        intent.putExtra(
                                                EXTRA_RESTART_ACTIVITY_CLASS,
                                                restartActivityClass);
                                        intent.putExtra(
                                                EXTRA_SHOW_ERROR_DETAILS, showErrorDetails);
                                        intent.setFlags(
                                                Intent.FLAG_ACTIVITY_NEW_TASK
                                                        | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                                        intent.putExtra(
                                                EXTRA_EXCEPTION_TYPE,
                                                throwable.getClass().getName());
                                        intent.putExtra(
                                                EXTRA_THREAD_INFO,
                                                thread.getName()
                                                        + " (ID:"
                                                        + thread.getId()
                                                        + ")");
                                        intent.putExtra(
                                                EXTRA_CRASH_CONTEXT,
                                                getCrashContext(application));
                                        application.startActivity(intent);
                                    }
                                }
                                final Activity lastActivity = lastActivityCreated.get();
                                if (lastActivity != null) {
                                    lastActivity.finish();
                                    lastActivityCreated.clear();
                                }
                                killCurrentProcess();
                            });
                    
                    application.registerActivityLifecycleCallbacks(
                            new Application.ActivityLifecycleCallbacks() {
                                int currentlyStartedActivities = 0;

                                @Override
                                public void onActivityCreated(
                                        Activity activity, Bundle savedInstanceState) {
                                    if (activity.getClass() != errorActivityClass) {
                                        lastActivityCreated = new WeakReference<>(activity);
                                    }
                                }

                                @Override
                                public void onActivityStarted(Activity activity) {
                                    currentlyStartedActivities++;
                                    isInBackground = (currentlyStartedActivities == 0);
                                }

                                @Override
                                public void onActivityResumed(Activity activity) {}

                                @Override
                                public void onActivityPaused(Activity activity) {}

                                @Override
                                public void onActivityStopped(Activity activity) {
                                    currentlyStartedActivities--;
                                    isInBackground = (currentlyStartedActivities == 0);
                                }

                                @Override
                                public void onActivitySaveInstanceState(
                                        Activity activity, Bundle outState) {}

                                @Override
                                public void onActivityDestroyed(Activity activity) {}
                            });

                    Log.i(TAG, "crashmanager has been installed.");
                }
            }
        } catch (Throwable t) {
            Log.e(
                    TAG,
                    "An unknown error occurred while installing crashmanager, it may not have been properly initialized. Please report this as a bug if needed.",
                    t);
        }
    }

    public static boolean isShowErrorDetailsFromIntent(Intent intent) {
        return intent.getBooleanExtra(CrashManager.EXTRA_SHOW_ERROR_DETAILS, true);
    }

    private static int getColorPrimary(Context context) {
        TypedValue typedValue = new TypedValue();
        context.getTheme()
                .resolveAttribute(androidx.appcompat.R.attr.colorPrimary, typedValue, true);
        return typedValue.data;
    }

    public static SpannableStringBuilder getStackTraceFromIntent(Context context, Intent intent) {
        SpannableStringBuilder sb = new SpannableStringBuilder();
        int titleColor = getColorPrimary(context);

        // 异常类型
        appendWithStyle(sb, "\n[Exception Type]\n", new StyleSpan(Typeface.BOLD), titleColor);
        appendWithStyle(
                sb,
                intent.getStringExtra(EXTRA_EXCEPTION_TYPE) + "\n\n",
                new ForegroundColorSpan(ContextCompat.getColor(context, R.color.warning)),
                0);

        // 线程信息
        appendWithStyle(sb, "[Thread Info]\n", new StyleSpan(Typeface.BOLD), titleColor);
        sb.append(intent.getStringExtra(EXTRA_THREAD_INFO)).append("\n\n");

        // 崩溃上下文
        appendWithStyle(sb, "[Crash Context]\n", new StyleSpan(Typeface.ITALIC), titleColor);
        sb.append(intent.getStringExtra(EXTRA_CRASH_CONTEXT)).append("\n\n");

        // 堆栈跟踪
        appendWithStyle(sb, "[Stack Trace]\n", new StyleSpan(Typeface.BOLD), titleColor);
        sb.append(intent.getStringExtra(EXTRA_STACK_TRACE));

        return sb;
    }

    private static String getCrashContext(Context context) {
        return "Application: " + context.getPackageName() + 
               "\nTime: " + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.US).format(new Date());
    }

    public static SpannableStringBuilder getAllErrorDetailsFromIntent(
            Context context, Intent intent) {
        SpannableStringBuilder errorDetails = new SpannableStringBuilder();
        int sectionColor = ContextCompat.getColor(context, R.color.section_title);
        int highlightColor = ContextCompat.getColor(context, R.color.highlight);

        // 基础信息
        appendSection(errorDetails, "Crash Time:", sectionColor);
        errorDetails.append(
                new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.US).format(new Date()));
        errorDetails.append("\n\n");

        // 应用版本
        appendSection(errorDetails, "App Version", sectionColor);
        try {
            PackageInfo packageInfo =
                    context.getPackageManager().getPackageInfo(context.getPackageName(), 0);
            appendKeyValue(errorDetails, "Name", packageInfo.versionName, highlightColor);
            long versionCode = Build.VERSION.SDK_INT >= 28
                    ? packageInfo.getLongVersionCode()
                    : packageInfo.versionCode;
            appendKeyValue(
                    errorDetails,
                    "Code",
                    String.valueOf(versionCode),
                    highlightColor);
        } catch (Throwable t) {
            errorDetails.append("Version info unavailable\n");
        }
        errorDetails.append("\n");

        // 设备信息
        appendSection(errorDetails, "Device Info", sectionColor);
        appendKeyValue(errorDetails, "Model", Build.MODEL, highlightColor);
        appendKeyValue(errorDetails, "Android", Build.VERSION.RELEASE + " (SDK " + Build.VERSION.SDK_INT + ")", highlightColor);
        errorDetails.append("\n");

        // 堆栈信息
        appendSection(errorDetails, "Crash Details", sectionColor);
        errorDetails.append(getStackTraceFromIntent(context, intent));

        return errorDetails;
    }

    private static void appendSection(SpannableStringBuilder sb, String title, int color) {
        int start = sb.length();
        sb.append(title).append("\n");
        sb.setSpan(
                new StyleSpan(Typeface.BOLD), start, sb.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        sb.setSpan(
                new ForegroundColorSpan(color),
                start,
                sb.length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    }

    private static void appendKeyValue(
            SpannableStringBuilder sb, String key, String value, int color) {
        sb.append("• ").append(key).append(": ");
        int valueStart = sb.length();
        sb.append(value).append("\n");
        sb.setSpan(
                new ForegroundColorSpan(color),
                valueStart,
                sb.length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    }

    private static void appendWithStyle(
            SpannableStringBuilder sb, String text, Object span, int color) {
        int start = sb.length();
        sb.append(text);
        if (color != 0) {
            sb.setSpan(
                    new ForegroundColorSpan(color),
                    start,
                    sb.length(),
                    Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
        if (span != null) {
            sb.setSpan(span, start, sb.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
    }

    @SuppressWarnings("unchecked")
    public static Class<? extends Activity> getRestartActivityClassFromIntent(Intent intent) {
        Serializable serializedClass =
                intent.getSerializableExtra(CrashManager.EXTRA_RESTART_ACTIVITY_CLASS);

        if (serializedClass != null && serializedClass instanceof Class) {
            return (Class<? extends Activity>) serializedClass;
        } else {
            return null;
        }
    }

    public static void restartApplicationWithIntent(Activity activity, Intent intent) {
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        activity.finish();
        activity.startActivity(intent);
        killCurrentProcess();
    }

    public static void closeApplication(Activity activity) {
        activity.finish();
        killCurrentProcess();
    }

    // Getters and setters
    public static boolean isLaunchErrorActivityWhenInBackground() {
        return launchErrorActivityWhenInBackground;
    }

    public static void setLaunchErrorActivityWhenInBackground(
            boolean launchErrorActivityWhenInBackground) {
        CrashManager.launchErrorActivityWhenInBackground = launchErrorActivityWhenInBackground;
    }

    public static boolean isShowErrorDetails() {
        return showErrorDetails;
    }

    public static void setShowErrorDetails(boolean showErrorDetails) {
        CrashManager.showErrorDetails = showErrorDetails;
    }

    public static boolean isEnableAppRestart() {
        return enableAppRestart;
    }

    public static void setEnableAppRestart(boolean enableAppRestart) {
        CrashManager.enableAppRestart = enableAppRestart;
    }

    public static Class<? extends Activity> getErrorActivityClass() {
        return errorActivityClass;
    }

    public static void setErrorActivityClass(Class<? extends Activity> errorActivityClass) {
        CrashManager.errorActivityClass = errorActivityClass;
    }

    public static Class<? extends Activity> getRestartActivityClass() {
        return restartActivityClass;
    }

    public static void setRestartActivityClass(Class<? extends Activity> restartActivityClass) {
        CrashManager.restartActivityClass = restartActivityClass;
    }

    private static boolean isStackTraceLikelyConflictive(
            Throwable throwable, Class<? extends Activity> activityClass) {
        do {
            StackTraceElement[] stackTrace = throwable.getStackTrace();
            for (StackTraceElement element : stackTrace) {
                if ((element.getClassName().equals("android.app.ActivityThread")
                                && element.getMethodName().equals("handleBindApplication"))
                        || element.getClassName().equals(activityClass.getName())) {
                    return true;
                }
            }
        } while ((throwable = throwable.getCause()) != null);
        return false;
    }

    private static Class<? extends Activity> guessRestartActivityClass(Context context) {
        Class<? extends Activity> resolvedActivityClass;

        resolvedActivityClass = CrashManager.getRestartActivityClassWithIntentFilter(context);

        if (resolvedActivityClass == null) {
            resolvedActivityClass = getLauncherActivity(context);
        }

        return resolvedActivityClass;
    }

    @SuppressWarnings("unchecked")
    private static Class<? extends Activity> getRestartActivityClassWithIntentFilter(
            Context context) {
        List<ResolveInfo> resolveInfos =
                context.getPackageManager()
                        .queryIntentActivities(
                                new Intent().setAction(INTENT_ACTION_RESTART_ACTIVITY),
                                PackageManager.GET_RESOLVED_FILTER);

        if (resolveInfos != null && !resolveInfos.isEmpty()) {
            ResolveInfo resolveInfo = resolveInfos.get(0);
            try {
                return (Class<? extends Activity>) Class.forName(resolveInfo.activityInfo.name);
            } catch (ClassNotFoundException e) {
                Log.e(
                        TAG,
                        "Failed when resolving the restart activity class via intent filter, stack trace follows!",
                        e);
            }
        }

        return null;
    }

    @SuppressWarnings("unchecked")
    private static Class<? extends Activity> getLauncherActivity(Context context) {
        Intent intent =
                context.getPackageManager().getLaunchIntentForPackage(context.getPackageName());
        if (intent != null && intent.getComponent() != null) {
            try {
                return (Class<? extends Activity>)
                        Class.forName(intent.getComponent().getClassName());
            } catch (ClassNotFoundException e) {
                Log.e(
                        TAG,
                        "Failed when resolving the restart activity class via getLaunchIntentForPackage, stack trace follows!",
                        e);
            }
        }

        return null;
    }

    private static Class<? extends Activity> guessErrorActivityClass(Context context) {
        Class<? extends Activity> resolvedActivityClass;

        resolvedActivityClass = CrashManager.getErrorActivityClassWithIntentFilter(context);

        if (resolvedActivityClass == null) {
            resolvedActivityClass = ErrorActivity.class;
        }

        return resolvedActivityClass;
    }

    @SuppressWarnings("unchecked")
    private static Class<? extends Activity> getErrorActivityClassWithIntentFilter(
            Context context) {
        List<ResolveInfo> resolveInfos =
                context.getPackageManager()
                        .queryIntentActivities(
                                new Intent().setAction(INTENT_ACTION_ERROR_ACTIVITY),
                                PackageManager.GET_RESOLVED_FILTER);

        if (resolveInfos != null && !resolveInfos.isEmpty()) {
            ResolveInfo resolveInfo = resolveInfos.get(0);
            try {
                return (Class<? extends Activity>) Class.forName(resolveInfo.activityInfo.name);
            } catch (ClassNotFoundException e) {
                Log.e(
                        TAG,
                        "通过意图过滤器解析错误活动类失败，堆栈跟踪如下！",
                        e);
            }
        }

        return null;
    }

    private static void killCurrentProcess() {
        android.os.Process.killProcess(android.os.Process.myPid());
        System.exit(10);
    }
}