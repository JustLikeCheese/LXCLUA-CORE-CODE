package com.difierline.lua.material.floatingactionbutton

import android.content.Context
import android.content.res.ColorStateList
import android.graphics.drawable.Drawable
import android.util.AttributeSet
import android.util.TypedValue
import android.view.LayoutInflater
import android.view.View
import android.view.View.OnClickListener
import android.view.View.OnLongClickListener
import android.app.Activity
import android.widget.FrameLayout
import com.google.android.material.card.MaterialCardView
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.difierline.lua.material.R
import com.difierline.lua.material.Material
import com.difierline.lua.utils.SizeUtil
import com.difierline.lua.utils.ColorUtil

class FloatingActionButton @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : FrameLayout(context, attrs, defStyleAttr) {

    private val card: MaterialCardView = MaterialCardView(context)
    private val fab: FloatingActionButton = FloatingActionButton(context)
    private val fabAnimator = FloatingActionButtonAnimator(card)
    
    var backgroundTint: ColorStateList? = null
        set(value) {
            fab.backgroundTintList = value
        }

    init {

        addView(card)
        card.addView(fab)
        
        fabAnimator.init()
        
        card.setStrokeWidth(SizeUtil.dp2px(context as Activity, 1))
        card.setStrokeColor(ColorUtil.getColor2(context as Activity, R.color.colorFabBorder))
        card.setCardElevation(0f)

        var attributes = context.obtainStyledAttributes(
            attrs, R.styleable.FloatingActionButton
        )
        backgroundTint = attributes.getColorStateList(R.styleable.FloatingActionButton_backgroundTint)
        attributes.recycle()
        
        if (Material.isMaterial3Enabled(context)) {
            card.setRadius(SizeUtil.dp2px(context, 16).toFloat())
            fab.setBackgroundTintList(ColorStateList.valueOf(ColorUtil.getColor(context as Activity, "colorPrimaryContainer")))
        } else {
            card.setRadius(SizeUtil.dp2px(context, 360).toFloat())
            fab.setBackgroundTintList(ColorStateList.valueOf(ColorUtil.getColor(context as Activity, "colorPrimary")))
        }

    }

    fun setImageDrawable(drawable: Drawable) {
        fab.setImageDrawable(drawable)
    }
    
    fun setImageResource(drawable: Int) {
        fab.setImageResource(drawable)
    }
    
    fun setBackgroundColor(colorStateList: ColorStateList) {
        fab.backgroundTintList = colorStateList
    }

    fun show() {
        fabAnimator.show()
    }

    fun hide() {
        fabAnimator.hide()
    }
    
    override fun setOnClickListener(onClickListener: OnClickListener?) {
        fab.setOnClickListener(onClickListener)
    }
    
    override fun setOnLongClickListener(onLongClickListener: OnLongClickListener?) {
        fab.setOnLongClickListener(onLongClickListener)
    }
    
}