package com.difierline.lua.utils

import android.content.Context
import android.util.TypedValue
import androidx.annotation.ColorInt
import com.google.android.material.color.MaterialColors
import com.difierline.lua.material.Material
import android.graphics.Color

object ThemeUtil {

    fun isDarkTheme(context: Context): Boolean {
        val typedValue = TypedValue()
        context.theme.resolveAttribute(android.R.attr.isLightTheme, typedValue, true)
        return typedValue.data == 0
    }

    @ColorInt
    fun getThemeColor(context: Context, attrResId: Int): Int {
        return try {
            MaterialColors.getColor(context, attrResId, Color.BLACK)
        } catch (e: Exception) {
            Color.BLACK
        }
    }

    @ColorInt
    fun getColorPrimary(context: Context): Int {
        return try {
            MaterialColors.getColor(context, android.R.attr.colorPrimary, Color.BLACK)
        } catch (e: Exception) {
            Color.parseColor("#6750A4")
        }
    }

    @ColorInt
    fun getColorPrimaryVariant(context: Context): Int {
        return getThemeColor(context, com.google.android.material.R.attr.colorPrimaryVariant)
    }

    @ColorInt
    fun getColorSecondary(context: Context): Int {
        return try {
            MaterialColors.getColor(context, android.R.attr.colorSecondary, Color.BLACK)
        } catch (e: Exception) {
            Color.parseColor("#625B71")
        }
    }

    @ColorInt
    fun getColorSecondaryVariant(context: Context): Int {
        return getThemeColor(context, com.google.android.material.R.attr.colorSecondaryVariant)
    }

    @ColorInt
    fun getColorSurface(context: Context): Int {
        return try {
            MaterialColors.getColor(context, android.R.attr.colorBackground, Color.WHITE)
        } catch (e: Exception) {
            Color.WHITE
        }
    }

    @ColorInt
    fun getColorBackground(context: Context): Int {
        return getThemeColor(context, android.R.attr.colorBackground)
    }

    @ColorInt
    fun getColorError(context: Context): Int {
        return try {
            MaterialColors.getColor(context, android.R.attr.colorError, Color.RED)
        } catch (e: Exception) {
            Color.parseColor("#B3261E")
        }
    }

    @ColorInt
    fun getColorOnPrimary(context: Context): Int {
        return getThemeColor(context, com.google.android.material.R.attr.colorOnPrimary)
    }

    @ColorInt
    fun getColorOnSecondary(context: Context): Int {
        return getThemeColor(context, com.google.android.material.R.attr.colorOnSecondary)
    }

    @ColorInt
    fun getColorOnSurface(context: Context): Int {
        return getThemeColor(context, com.google.android.material.R.attr.colorOnSurface)
    }

    @ColorInt
    fun getColorOnBackground(context: Context): Int {
        return getThemeColor(context, com.google.android.material.R.attr.colorOnBackground)
    }

    @ColorInt
    fun getColorOnError(context: Context): Int {
        return getThemeColor(context, com.google.android.material.R.attr.colorOnError)
    }

    fun getMaterialVersion(context: Context): MaterialVersion {
        return when {
            Material.isMaterial3Enabled(context) -> MaterialVersion.MATERIAL_3
            Material.isMaterial2Enabled(context) -> MaterialVersion.MATERIAL_2
            else -> MaterialVersion.MATERIAL_1
        }
    }

    @ColorInt
    fun getColorByMaterialVersion(context: Context, material2Color: Int, material3Color: Int): Int {
        return if (Material.isMaterial3Enabled(context)) material3Color else material2Color
    }

    enum class MaterialVersion {
        MATERIAL_1,
        MATERIAL_2,
        MATERIAL_3
    }
}
