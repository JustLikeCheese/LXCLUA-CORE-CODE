package com.difierline.lua.utils

import android.content.Context

object SizeUtil {
   
    fun dp2px(context: Context, dp: Int): Int {
        return (dp * context.resources.displayMetrics.density + 0.5f).toInt()
    }
    
}