package com.difierline.lua.lxclua.utils

import android.app.Activity
import android.content.Context
import android.content.res.Configuration
import android.graphics.Color
import android.graphics.Point
import android.os.Build
import android.util.DisplayMetrics
import android.util.TypedValue
import android.view.View
import android.view.Window
import android.view.WindowInsets
import android.view.WindowManager
import androidx.annotation.ColorInt
import androidx.annotation.Dimension
import androidx.annotation.Keep
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsControllerCompat
import java.lang.reflect.Method

@Keep
object UiUtil {

    @JvmStatic
    fun isNightMode(activity: Activity): Boolean {
        val currentNightMode = activity.resources.configuration.uiMode and Configuration.UI_MODE_NIGHT_MASK
        return currentNightMode == Configuration.UI_MODE_NIGHT_YES
    }

    @JvmStatic
    fun getStatusBarHeight(context: Context): Int {
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            val resourceId = context.resources.getIdentifier("status_bar_height", "dimen", "android")
            if (resourceId > 0) context.resources.getDimensionPixelSize(resourceId) else 0
        } else {
            0
        }
    }

    @JvmStatic
    fun getActionBarSize(context: Context): Int {
        val typedValue = TypedValue()
        return if (context.theme.resolveAttribute(android.R.attr.actionBarSize, typedValue, true)) {
            TypedValue.complexToDimensionPixelSize(
                typedValue.data,
                context.resources.displayMetrics
            )
        } else {
            0
        }
    }

    @JvmStatic
    fun getCollapsingToolbarLayoutMediumSize(context: Context): Int {
        val typedValue = TypedValue()
        // 首先尝试获取 Material 组件中的属性
        val attrResourceId = context.resources.getIdentifier(
            "collapsingToolbarLayoutMediumSize", 
            "attr", 
            "com.google.android.material"
        )
        
        return if (attrResourceId != 0 && 
            context.theme.resolveAttribute(attrResourceId, typedValue, true)) {
            TypedValue.complexToDimensionPixelSize(
                typedValue.data,
                context.resources.displayMetrics
            )
        } else {
            // 如果 Material 组件属性不存在，尝试从当前主题中查找
            val fallbackAttrId = context.resources.getIdentifier(
                "collapsingToolbarLayoutMediumSize", 
                "attr", 
                context.packageName
            )
            
            if (fallbackAttrId != 0 && 
                context.theme.resolveAttribute(fallbackAttrId, typedValue, true)) {
                TypedValue.complexToDimensionPixelSize(
                    typedValue.data,
                    context.resources.displayMetrics
                )
            } else {
                0 // 属性不存在，返回0
            }
        }
    }

    @JvmStatic
    fun getNavigationBarHeight(context: Context): Int {
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            val resourceId = context.resources.getIdentifier("navigation_bar_height", "dimen", "android")
            if (resourceId > 0) context.resources.getDimensionPixelSize(resourceId) else 0
        } else {
            0
        }
    }

    @JvmStatic
    @Dimension(unit = Dimension.PX)
    fun dp2px(context: Context, @Dimension(unit = Dimension.DP) dp: Float): Int {
        return TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            dp,
            context.resources.displayMetrics
        ).toInt()
    }

    @JvmStatic
    @Dimension(unit = Dimension.PX)
    fun dp2pxWithDensity(density: Float, @Dimension(unit = Dimension.DP) dp: Float): Int {
        return (dp * density + 0.5f).toInt()
    }

    @JvmStatic
    @Dimension(unit = Dimension.DP)
    fun px2dp(context: Context, @Dimension(unit = Dimension.PX) px: Float): Float {
        return px / context.resources.displayMetrics.density
    }

    @JvmStatic
    @Dimension(unit = Dimension.DP)
    fun px2dpWithDensity(density: Float, @Dimension(unit = Dimension.PX) px: Float): Float {
        return px / density
    }

    @JvmStatic
    @Dimension(unit = Dimension.PX)
    fun sp2px(context: Context, @Dimension(unit = Dimension.SP) sp: Float): Int {
        return TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_SP,
            sp,
            context.resources.displayMetrics
        ).toInt()
    }

    @JvmStatic
    @Dimension(unit = Dimension.PX)
    fun sp2pxWithDensity(scaledDensity: Float, @Dimension(unit = Dimension.SP) sp: Float): Int {
        return (sp * scaledDensity + 0.5f).toInt()
    }

    @JvmStatic
    fun getScreenWidth(context: Context): Int {
        val windowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            val windowMetrics = windowManager.currentWindowMetrics
            windowMetrics.bounds.width()
        } else {
            val displayMetrics = DisplayMetrics()
            val display = windowManager.defaultDisplay
            display.getMetrics(displayMetrics)
            displayMetrics.widthPixels
        }
    }

    @JvmStatic
    fun getScreenHeight(context: Context): Int {
        val windowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            val windowMetrics = windowManager.currentWindowMetrics
            windowMetrics.bounds.height()
        } else {
            val displayMetrics = DisplayMetrics()
            val display = windowManager.defaultDisplay
            display.getMetrics(displayMetrics)
            displayMetrics.heightPixels
        }
    }

    @JvmStatic
    fun getRealScreenHeight(context: Context): Int {
        val windowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            val windowMetrics = windowManager.currentWindowMetrics
            windowMetrics.bounds.height()
        } else {
            val displayMetrics = DisplayMetrics()
            val display = windowManager.defaultDisplay
            display.getMetrics(displayMetrics)
            displayMetrics.heightPixels
        }
    }

    @JvmStatic
    fun isLandscape(context: Context): Boolean {
        return context.resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE
    }

    @JvmStatic
    fun isPortrait(context: Context): Boolean {
        return context.resources.configuration.orientation == Configuration.ORIENTATION_PORTRAIT
    }

    @JvmStatic
    fun isTablet(context: Context): Boolean {
        return (context.resources.configuration.screenLayout and Configuration.SCREENLAYOUT_SIZE_MASK) >= Configuration.SCREENLAYOUT_SIZE_LARGE
    }

    @JvmStatic
    fun setLightStatusBar(activity: Activity, light: Boolean) {
        val window = activity.window
        val decorView = window.decorView
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            @Suppress("DEPRECATION")
            var flags = decorView.systemUiVisibility
            @Suppress("DEPRECATION")
            flags = if (light) {
                flags or View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
            } else {
                flags and View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR.inv()
            }
            @Suppress("DEPRECATION")
            decorView.systemUiVisibility = flags
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            WindowCompat.getInsetsController(window, decorView).isAppearanceLightStatusBars = light
        }
    }

    @JvmStatic
    fun setLightNavigationBar(activity: Activity, light: Boolean) {
        val window = activity.window
        val decorView = window.decorView
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            @Suppress("DEPRECATION")
            var flags = decorView.systemUiVisibility
            @Suppress("DEPRECATION")
            flags = if (light) {
                flags or View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR
            } else {
                flags and View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR.inv()
            }
            @Suppress("DEPRECATION")
            decorView.systemUiVisibility = flags
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            WindowCompat.getInsetsController(window, decorView).isAppearanceLightNavigationBars = light
        }
    }

    @JvmStatic
    fun setStatusBarColor(activity: Activity, @ColorInt color: Int) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            @Suppress("DEPRECATION")
            activity.window.statusBarColor = color
        }
    }

    @JvmStatic
    fun setNavigationBarColor(activity: Activity, @ColorInt color: Int) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            @Suppress("DEPRECATION")
            activity.window.navigationBarColor = color
        }
    }

    @JvmStatic
    fun setFullScreen(activity: Activity, fullScreen: Boolean) {
        val window = activity.window
        val decorView = window.decorView
        
        if (fullScreen) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                @Suppress("DEPRECATION")
                window.setDecorFitsSystemWindows(false)
                val controller = WindowCompat.getInsetsController(window, decorView)
                controller.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                controller.systemBarsBehavior = WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            } else {
                @Suppress("DEPRECATION")
                decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_FULLSCREEN
                        or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY)
            }
        } else {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                @Suppress("DEPRECATION")
                window.setDecorFitsSystemWindows(true)
                WindowCompat.getInsetsController(window, decorView).show(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
            } else {
                @Suppress("DEPRECATION")
                decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_VISIBLE
            }
        }
    }

    @JvmStatic
    fun isFullScreen(activity: Activity): Boolean {
        val decorView = activity.window.decorView
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            val windowInsets = decorView.rootWindowInsets
            windowInsets?.isVisible(WindowInsets.Type.statusBars()) == false
        } else {
            @Suppress("DEPRECATION")
            (decorView.systemUiVisibility and View.SYSTEM_UI_FLAG_FULLSCREEN) == View.SYSTEM_UI_FLAG_FULLSCREEN
        }
    }

    @JvmStatic
    fun getDisplayMetrics(context: Context): DisplayMetrics {
        return context.resources.displayMetrics
    }

    @JvmStatic
    fun getDensity(context: Context): Float {
        return context.resources.displayMetrics.density
    }

    @JvmStatic
    fun getDensityDpi(context: Context): Int {
        return context.resources.displayMetrics.densityDpi
    }

    @JvmStatic
    fun getScaledDensity(context: Context): Float {
        return context.resources.displayMetrics.scaledDensity
    }

    @JvmStatic
    fun isColorDark(@ColorInt color: Int): Boolean {
        val darkness = 1 - (0.299 * Color.red(color) + 0.587 * Color.green(color) + 0.114 * Color.blue(color)) / 255
        return darkness >= 0.5
    }

    @JvmStatic
    fun getContrastColor(@ColorInt backgroundColor: Int): Int {
        return if (isColorDark(backgroundColor)) {
            Color.WHITE
        } else {
            Color.BLACK
        }
    }

    @JvmStatic
    fun adjustAlpha(@ColorInt color: Int, factor: Float): Int {
        val alpha = (Color.alpha(color) * factor).toInt()
        val red = Color.red(color)
        val green = Color.green(color)
        val blue = Color.blue(color)
        return Color.argb(alpha, red, green, blue)
    }

    @JvmStatic
    fun getColorWithAlpha(@ColorInt color: Int, alpha: Int): Int {
        return Color.argb(alpha, Color.red(color), Color.green(color), Color.blue(color))
    }

    @JvmStatic
    fun getScreenAspectRatio(context: Context): Float {
        val width = getScreenWidth(context)
        val height = getScreenHeight(context)
        return width.toFloat() / height.toFloat()
    }

    @JvmStatic
    fun isRtl(context: Context): Boolean {
        return context.resources.configuration.layoutDirection == View.LAYOUT_DIRECTION_RTL
    }

    @JvmStatic
    fun getSmallestScreenWidthDp(context: Context): Int {
        return context.resources.configuration.smallestScreenWidthDp
    }

    @JvmStatic
    fun getScreenWidthDp(context: Context): Int {
        return context.resources.configuration.screenWidthDp
    }

    @JvmStatic
    fun getScreenHeightDp(context: Context): Int {
        return context.resources.configuration.screenHeightDp
    }

    @JvmStatic
    fun hasNotch(activity: Activity): Boolean {
        // 刘海屏检测（部分品牌可能需要特殊处理）
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            val windowInsets = activity.window.decorView.rootWindowInsets
            windowInsets?.displayCutout != null
        } else {
            // 对于Android P以下版本，可以尝试检测特定品牌的刘海屏
            isBrandSpecificNotch(activity)
        }
    }

    private fun isBrandSpecificNotch(activity: Activity): Boolean {
        return try {
            val resources = activity.resources
            val resourceId = resources.getIdentifier("status_bar_height", "dimen", "android")
            val statusBarHeight = resources.getDimensionPixelSize(resourceId)
            statusBarHeight > dp2px(activity, 24f) // 如果状态栏高度异常高，可能是有刘海
        } catch (e: Exception) {
            false
        }
    }

    /**
     * 修改应用的DPI缩放比例
     * 注意：此方法需要系统权限或仅在特定环境下有效
     */
    @JvmStatic
    fun setAppDensity(activity: Activity, density: Float) {
        val resources = activity.resources
        val displayMetrics = resources.displayMetrics
        val configuration = resources.configuration
        
        // 修改显示度量
        displayMetrics.density = density
        @Suppress("DEPRECATION")
        displayMetrics.scaledDensity = density
        displayMetrics.densityDpi = (density * 160).toInt()
        
        // 修改配置
        configuration.densityDpi = (density * 160).toInt()
        
        // 应用更改
        @Suppress("DEPRECATION")
        resources.updateConfiguration(configuration, displayMetrics)
    }
    
    /**
     * 修改应用的字体缩放比例
     */
    @JvmStatic
    fun setAppScaledDensity(activity: Activity, scaledDensity: Float) {
        val resources = activity.resources
        val displayMetrics = resources.displayMetrics
        
        // 修改显示度量的缩放密度
        @Suppress("DEPRECATION")
        displayMetrics.scaledDensity = scaledDensity
        
        // 应用更改
        @Suppress("DEPRECATION")
        resources.updateConfiguration(resources.configuration, displayMetrics)
    }
    
    /**
     * 重置应用的DPI设置为系统默认
     */
    @JvmStatic
    fun resetAppDensity(activity: Activity) {
        val resources = activity.resources
        val displayMetrics = resources.displayMetrics
        val configuration = resources.configuration
        
        // 获取系统默认的显示度量
        val systemDisplayMetrics = resources.displayMetrics
        val systemDensity = systemDisplayMetrics.density
        val systemScaledDensity = systemDisplayMetrics.scaledDensity
        val systemDensityDpi = systemDisplayMetrics.densityDpi
        
        // 恢复为系统默认值
        displayMetrics.density = systemDensity
        displayMetrics.scaledDensity = systemScaledDensity
        displayMetrics.densityDpi = systemDensityDpi
        
        // 恢复配置
        configuration.densityDpi = systemDensityDpi
        
        // 应用更改
        @Suppress("DEPRECATION")
        resources.updateConfiguration(configuration, displayMetrics)
    }
    
    /**
     * 获取当前应用的密度比例
     */
    @JvmStatic
    fun getAppDensity(context: Context): Float {
        return context.resources.displayMetrics.density
    }
    
    /**
     * 获取当前应用的字体缩放比例
     */
    @JvmStatic
    fun getAppScaledDensity(context: Context): Float {
        return context.resources.displayMetrics.scaledDensity
    }
    
    /**
     * 获取当前应用的DPI值
     */
    @JvmStatic
    fun getAppDensityDpi(context: Context): Int {
        return context.resources.displayMetrics.densityDpi
    }
    
    /**
     * 计算指定DPI值对应的密度比例
     */
    @JvmStatic
    fun calculateDensityFromDpi(dpi: Int): Float {
        return dpi / 160f
    }
    
    /**
     * 计算指定密度比例对应的DPI值
     */
    @JvmStatic
    fun calculateDpiFromDensity(density: Float): Int {
        return (density * 160).toInt()
    }
    
    /**
     * 检查是否支持修改系统DPI（需要系统权限）
     */
    @JvmStatic
    fun isSystemDensityModificationSupported(): Boolean {
        return try {
            // 尝试通过反射检查是否有修改系统DPI的权限
            val windowManagerClass = Class.forName("android.view.WindowManagerGlobal")
            val getWindowManagerServiceMethod = windowManagerClass.getDeclaredMethod("getWindowManagerService")
            getWindowManagerServiceMethod.isAccessible = true
            true
        } catch (e: Exception) {
            false
        }
    }
    
    /**
     * 尝试修改系统DPI（需要系统权限）
     * 注意：此方法需要系统级权限，普通应用无法使用
     */
    @JvmStatic
    @Throws(Exception::class)
    fun setSystemDensity(dpi: Int) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1) {
            throw UnsupportedOperationException("系统DPI修改需要Android 4.2及以上版本")
        }
        
        try {
            // 通过反射调用系统服务修改DPI
            val windowManagerClass = Class.forName("android.view.WindowManagerGlobal")
            val getWindowManagerServiceMethod = windowManagerClass.getDeclaredMethod("getWindowManagerService")
            getWindowManagerServiceMethod.isAccessible = true
            val windowManagerService = getWindowManagerServiceMethod.invoke(null)
            
            val setForcedDisplayDensityMethod = windowManagerService.javaClass.getMethod(
                "setForcedDisplayDensity", 
                Int::class.javaPrimitiveType, 
                Int::class.javaPrimitiveType
            )
            
            // 主显示屏的ID通常是0
            setForcedDisplayDensityMethod.invoke(windowManagerService, 0, dpi)
        } catch (e: Exception) {
            throw Exception("修改系统DPI失败，可能需要系统权限", e)
        }
    }
    
    /**
     * 获取系统DPI设置（需要系统权限）
     */
    @JvmStatic
    @Throws(Exception::class)
    fun getSystemDensity(): Int {
        try {
            // 通过反射调用系统服务获取DPI
            val windowManagerClass = Class.forName("android.view.WindowManagerGlobal")
            val getWindowManagerServiceMethod = windowManagerClass.getDeclaredMethod("getWindowManagerService")
            getWindowManagerServiceMethod.isAccessible = true
            val windowManagerService = getWindowManagerServiceMethod.invoke(null)
            
            val getBaseDisplayDensityMethod = windowManagerService.javaClass.getMethod(
                "getBaseDisplayDensity", 
                Int::class.javaPrimitiveType
            )
            
            // 主显示屏的ID通常是0
            return getBaseDisplayDensityMethod.invoke(windowManagerService, 0) as Int
        } catch (e: Exception) {
            throw Exception("获取系统DPI失败", e)
        }
    }
    
    /**
     * 根据DPI值获取对应的显示大小分类
     */
    @JvmStatic
    fun getDensityNameFromDpi(dpi: Int): String {
        return when {
            dpi <= 120 -> "ldpi"
            dpi <= 160 -> "mdpi"
            dpi <= 240 -> "hdpi"
            dpi <= 320 -> "xhdpi"
            dpi <= 480 -> "xxhdpi"
            dpi <= 640 -> "xxxhdpi"
            else -> "unknown"
        }
    }
    
    /**
     * 根据密度比例获取对应的显示大小分类
     */
    @JvmStatic
    fun getDensityNameFromDensity(density: Float): String {
        return getDensityNameFromDpi((density * 160).toInt())
    }
}