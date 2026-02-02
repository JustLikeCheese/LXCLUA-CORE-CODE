package com.difierline.lua.material.divider

import android.content.Context
import android.app.Activity
import android.util.AttributeSet
import android.view.View
import android.widget.FrameLayout
import com.google.android.material.divider.MaterialDivider
import com.difierline.lua.material.R
import com.difierline.lua.utils.ColorUtil

class MaterialDivider @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : MaterialDivider(context, attrs, defStyleAttr) {
    
    init {
        
        setDividerColor(ColorUtil.getColor2(context as Activity, R.color.colorBorder))
        
     }
    

}