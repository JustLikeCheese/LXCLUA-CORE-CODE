package com.difierline.lua.material.switches

import android.app.Activity
import android.graphics.Color
import android.content.Context
import android.content.res.ColorStateList
import android.util.AttributeSet
import android.view.Gravity
import android.view.ViewGroup.LayoutParams.MATCH_PARENT
import android.view.ViewGroup.LayoutParams.WRAP_CONTENT
import android.widget.Checkable
import android.widget.CompoundButton
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.widget.TextViewCompat
import com.google.android.material.color.MaterialColors
import com.google.android.material.shape.MaterialShapeDrawable
import com.google.android.material.textview.MaterialTextView
import com.difierline.lua.material.Material
import com.difierline.lua.material.R
import com.difierline.lua.material.foreground.ForegroundLinearLayout
import com.difierline.lua.material.foreground.foregroundCompat
import com.difierline.lua.utils.ResourcesUtil.getShapeAppearanceModelResource
import com.difierline.lua.utils.useAndRecycle
import com.difierline.lua.utils.RippleUtil
import com.difierline.lua.utils.ColorUtil

class MaterialSwitchBar @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = R.attr.materialSwitchBarStyle,
    defStyleRes: Int = if (Material.isMaterial3Enabled(context)) {
        R.style.Widget_Ripple_Material3_SwitchBar
    } else {
        R.style.Widget_Ripple_Material2_SwitchBar
    }
) : ForegroundLinearLayout(context, attrs, defStyleAttr), Checkable {

    companion object {
        private val CHECKED_STATE_SET = intArrayOf(android.R.attr.state_checked)
    }

    private val textView = MaterialTextView(context)
    private val switch: MaterialSwitch = MaterialSwitch(context)
    private var isChecked: Boolean = false
    var text: CharSequence?
        set(value) {
            textView.text = value
        }
        get() = textView.text
        
    var textColor: Int = ColorUtil.getColor(context as Activity, "colorOnPrimaryContainer") // 默认颜色
        set(value) {
            field = value
            textView.setTextColor(value) // 直接设置颜色值
        }
        get() = textView.currentTextColor // 获取当前文本颜色


    init {
        addView(textView, MATCH_PARENT, WRAP_CONTENT)
        addView(switch, WRAP_CONTENT, WRAP_CONTENT)
        gravity = Gravity.CENTER

        context.obtainStyledAttributes(
            attrs, R.styleable.MaterialSwitchBar, defStyleAttr, defStyleRes
        ).useAndRecycle {
            setPaddingRelative(
                0, 0, it.getDimensionPixelSize(R.styleable.MaterialSwitchBar_android_paddingEnd, 0), 0
            )

            textView.apply {
                (layoutParams as LayoutParams).weight = 1f
                TextViewCompat.setTextAppearance(
                    this, it.getResourceId(
                        R.styleable.MaterialSwitchBar_android_textAppearance, 0
                    )
                )
                setPaddingRelative(
                    it.getDimensionPixelSize(
                        R.styleable.MaterialSwitchBar_android_paddingStart, 0
                    ), it.getDimensionPixelSize(
                        R.styleable.MaterialSwitchBar_android_paddingTop, 0
                    ), 0, it.getDimensionPixelSize(
                        R.styleable.MaterialSwitchBar_android_paddingBottom, 0
                    )
                )
            }
            background = MaterialShapeDrawable(
                getShapeAppearanceModelResource(
                    context, it.getResourceId(R.styleable.MaterialSwitchBar_shapeAppearance, 0)
                )
            ).apply {
                fillColor = it.getColorStateList(R.styleable.MaterialSwitchBar_backgroundTint)
            }
        }

        foregroundCompat = RippleUtil.createRippleDrawableWithMask(context, background)

        if (!Material.isMaterial3Enabled(context)) {
            switch.trackTintList = AppCompatResources.getColorStateList(
                context, R.color.material_switch_bar_switch_track_tint_m2
            )
            switch.thumbTintList = AppCompatResources.getColorStateList(
                context, R.color.material_switch_bar_switch_thumb_tint_m2
            )
            if (textView.currentTextColor == ColorUtil.getColor(context as Activity, "colorOnPrimaryContainer")) {
                textView.setTextColor(
                    MaterialColors.getColorOrNull(
                        context, com.google.android.material.R.attr.colorOnPrimaryContainer
                    )!!
                )
            }
            switch.haloDrawable?.setColor(
                RippleUtil.convertToRippleColors(ColorStateList.valueOf(0x1f000000))
            )
        }

        switch.isClickable = false
        setOnClickListener {
            toggle()
        }
    }

    override fun drawableStateChanged() {
        super.drawableStateChanged()
        background.setState(drawableState)
        invalidate()
    }

    override fun onCreateDrawableState(extraSpace: Int): IntArray {
        val drawableState = super.onCreateDrawableState(extraSpace + 1)
        if (isChecked()) {
            mergeDrawableStates(drawableState, CHECKED_STATE_SET)
        }
        return drawableState
    }

    override fun setChecked(checked: Boolean) {
        isChecked = checked
        switch.isChecked = checked
    }

    override fun isChecked(): Boolean {
        return isChecked
    }

    override fun toggle() {
        isChecked = !isChecked
        switch.toggle()
    }

    fun setOnCheckedChangeListener(listener: CompoundButton.OnCheckedChangeListener) {
        switch.setOnCheckedChangeListener(listener)
    }

}