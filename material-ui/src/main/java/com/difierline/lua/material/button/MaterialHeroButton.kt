package com.difierline.lua.material.button

import android.app.Activity
import android.content.Context
import android.content.res.ColorStateList
import android.graphics.Color
import android.graphics.PorterDuff
import android.graphics.drawable.Drawable
import android.graphics.drawable.RippleDrawable
import android.util.AttributeSet
import android.view.LayoutInflater
import android.widget.FrameLayout
import androidx.appcompat.widget.AppCompatImageView
import androidx.appcompat.widget.AppCompatTextView
import com.google.android.material.card.MaterialCardView
import com.google.android.material.R as MDC_R
import com.difierline.lua.material.R
import com.difierline.lua.material.Material
import com.difierline.lua.utils.ColorUtil
import com.difierline.lua.utils.SizeUtil

class MaterialHeroButton @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0,
) : FrameLayout(context, attrs, defStyleAttr) {

    private val customLayout = LayoutInflater.from(context).inflate(R.layout.hero_button, this, false)
    private val imgView = customLayout.findViewById<AppCompatImageView>(R.id.hero_icon)
    private val textView = customLayout.findViewById<AppCompatTextView>(R.id.hero_text)
    private val activity = context as? Activity

    var icon: Drawable? = null
        set(value) {
            imgView.setImageDrawable(value)
            field = value
        }

    var text: String? = null
        set(value) {
            textView.text = value?.uppercase()
            field = value
        }

    var color: Int = Color.TRANSPARENT
        set(value) {
            imgView.setColorFilter(value, PorterDuff.Mode.SRC_IN)
            textView.setTextColor(value)
            field = value
        }

    var rippleColor: Int? = null
        set(value) {
            val rippleValue = value ?: run {
                activity?.let { ColorUtil.getColor2(it, R.color.colorRipple) } ?: Color.TRANSPARENT
            }
            
            // 只在第一次设置背景，后续直接修改颜色
            if (background !is RippleDrawable) {
                val attrs = intArrayOf(android.R.attr.selectableItemBackgroundBorderless)
                val typedArray = context.obtainStyledAttributes(attrs)
                val backgroundResource = typedArray.getResourceId(0, 0)
                setBackgroundResource(backgroundResource)
                typedArray.recycle()
            }
            
            val rippleDrawable = background as? RippleDrawable
            rippleDrawable?.setColor(ColorStateList.valueOf(rippleValue))
            field = value
        }

    init {
        addView(customLayout)
        val attributes = context.obtainStyledAttributes(attrs, R.styleable.MaterialHeroButton)
        
        // 设置图标
        icon = attributes.getDrawable(R.styleable.MaterialHeroButton_icon)
        
        // 设置文本
        text = attributes.getString(R.styleable.MaterialHeroButton_android_text)
        
        // 设置颜色
        val defaultColor = activity?.let { ColorUtil.getColor(it, "colorPrimary") } ?: Color.TRANSPARENT
        color = attributes.getInt(R.styleable.MaterialHeroButton_color, defaultColor)
        
        // 设置涟漪颜色
        val defaultRippleColor = activity?.let { ColorUtil.getColor2(it, R.color.colorRipple) } ?: Color.TRANSPARENT
        rippleColor = attributes.getInt(R.styleable.MaterialHeroButton_rippleColor, defaultRippleColor)
        
        attributes.recycle()
    }

}