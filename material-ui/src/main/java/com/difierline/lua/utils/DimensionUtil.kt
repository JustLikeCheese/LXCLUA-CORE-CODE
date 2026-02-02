package com.difierline.lua.utils

import android.content.Context
import android.util.DisplayMetrics
import android.util.TypedValue

/**
 * 尺寸单位转换工具类
 * 提供 dp、px、sp、pt 等单位之间的相互转换
 */
object DimensionUtil {

    /**
     * dp 转 px
     * @param context 上下文对象
     * @param dpValue dp 值
     * @return 转换后的 px 值
     */
    fun dp2px(context: Context, dpValue: Int): Int {
        return (dpValue * context.resources.displayMetrics.density + 0.5f).toInt()
    }

    /**
     * dp 转 px（支持浮点数）
     * @param context 上下文对象
     * @param dpValue dp 值
     * @return 转换后的 px 值
     */
    fun dp2px(context: Context, dpValue: Float): Int {
        return (dpValue * context.resources.displayMetrics.density + 0.5f).toInt()
    }

    /**
     * px 转 dp
     * @param context 上下文对象
     * @param pxValue px 值
     * @return 转换后的 dp 值
     */
    fun px2dp(context: Context, pxValue: Int): Float {
        return pxValue / context.resources.displayMetrics.density + 0.5f
    }

    /**
     * sp 转 px
     * @param context 上下文对象
     * @param spValue sp 值
     * @return 转换后的 px 值
     */
    fun sp2px(context: Context, spValue: Float): Float {
        return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP, spValue, context.resources.displayMetrics)
    }

    /**
     * px 转 sp
     * @param context 上下文对象
     * @param pxValue px 值
     * @return 转换后的 sp 值
     */
    fun px2sp(context: Context, pxValue: Float): Float {
        return pxValue / context.resources.displayMetrics.scaledDensity
    }

    /**
     * pt 转 px
     * @param context 上下文对象
     * @param ptValue pt 值
     * @return 转换后的 px 值
     */
    fun pt2px(context: Context, ptValue: Float): Float {
        return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_PT, ptValue, context.resources.displayMetrics)
    }

    /**
     * 获取屏幕宽度（px）
     * @param context 上下文对象
     * @return 屏幕宽度（px）
     */
    fun getScreenWidth(context: Context): Int {
        val displayMetrics = context.resources.displayMetrics
        return displayMetrics.widthPixels
    }

    /**
     * 获取屏幕高度（px）
     * @param context 上下文对象
     * @return 屏幕高度（px）
     */
    fun getScreenHeight(context: Context): Int {
        val displayMetrics = context.resources.displayMetrics
        return displayMetrics.heightPixels
    }

    /**
     * 获取状态栏高度
     * @param context 上下文对象
     * @return 状态栏高度（px）
     */
    fun getStatusBarHeight(context: Context): Int {
        val resourceId = context.resources.getIdentifier("status_bar_height", "dimen", "android")
        return if (resourceId > 0) {
            context.resources.getDimensionPixelSize(resourceId)
        } else {
            dp2px(context, 24)
        }
    }

    /**
     * 获取导航栏高度
     * @param context 上下文对象
     * @return 导航栏高度（px）
     */
    fun getNavigationBarHeight(context: Context): Int {
        val resourceId = context.resources.getIdentifier("navigation_bar_height", "dimen", "android")
        return if (resourceId > 0) {
            context.resources.getDimensionPixelSize(resourceId)
        } else {
            dp2px(context, 48)
        }
    }

    /**
     * 获取 ActionBar 高度
     * @param context 上下文对象
     * @return ActionBar 高度（px）
     */
    fun getActionBarSize(context: Context): Int {
        val typedValue = TypedValue()
        context.theme.resolveAttribute(android.R.attr.actionBarSize, typedValue, true)
        return TypedValue.complexToDimensionPixelSize(typedValue.data, context.resources.displayMetrics)
    }

    /**
     * 根据密度获取合适的尺寸
     * @param context 上下文对象
     * @param phoneSize 手机端尺寸（dp）
     * @param tabletSize 平板端尺寸（dp）
     * @return 适配后的尺寸（px）
     */
    fun getAdaptiveSize(context: Context, phoneSize: Int, tabletSize: Int): Int {
        val screenWidth = getScreenWidth(context)
        return if (screenWidth >= 600) {
            dp2px(context, tabletSize)
        } else {
            dp2px(context, phoneSize)
        }
    }
}
