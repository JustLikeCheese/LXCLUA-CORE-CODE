package com.difierline.lua.utils

import android.util.TypedValue
import android.view.View
import android.app.Activity
import android.content.Context
import com.google.android.material.color.MaterialColors
import androidx.core.content.ContextCompat

object ColorUtil {

    fun getColor(activity: Activity, colorName: String): Int {
        try {
            val colorAttr = activity.resources.getIdentifier(colorName, "attr", activity.packageName)
            if (colorAttr == 0) {
                val typedValue = TypedValue()
                activity.theme.resolveAttribute(android.R.attr.colorBackground, typedValue, true)
                return typedValue.data
            } else {
                return MaterialColors.getColor(activity.findViewById<View>(android.R.id.content), colorAttr)
            }
        } catch (e: Exception) {
            e.printStackTrace()
            return 0
        }
    }
    
    fun getColor2(activity: Activity, colorResId: Int): Int {
       return ContextCompat.getColor(activity, colorResId)
    }
    
}