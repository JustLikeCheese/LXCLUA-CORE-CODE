package com.difierline.lua.material.slider

import android.content.Context
import android.content.res.ColorStateList
import android.util.AttributeSet
import android.widget.FrameLayout
import com.google.android.material.slider.Slider
import com.google.android.material.slider.Slider.OnSliderTouchListener
import com.difierline.lua.material.R
import com.difierline.lua.material.Material
import com.difierline.lua.utils.DimensionUtil
import com.difierline.lua.utils.ThemeUtil

class MaterialSlider @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : FrameLayout(context, attrs, defStyleAttr) {

    private val slider: Slider = Slider(context).apply {
        layoutParams = LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT)
    }

    private var onChangeListener: ((value: Float, fromUser: Boolean) -> Unit)? = null

    init {
        addView(slider)
        applyDefaultStyles()
        parseAttributes(attrs)
        setupListeners()
    }

    private fun applyDefaultStyles() {
        slider.valueFrom = 0f
        slider.valueTo = 100f
        slider.value = 50f
        slider.stepSize = 1f
    }

    private fun setupListeners() {
        slider.addOnChangeListener { _, value, fromUser ->
            onChangeListener?.invoke(value, fromUser)
        }
    }

    private fun parseAttributes(attrs: AttributeSet?) {
        attrs?.let {
            val attributes = context.obtainStyledAttributes(it, R.styleable.MaterialSlider)
            slider.valueFrom = attributes.getFloat(R.styleable.MaterialSlider_valueFrom, 0f)
            slider.valueTo = attributes.getFloat(R.styleable.MaterialSlider_valueTo, 100f)
            slider.value = attributes.getFloat(R.styleable.MaterialSlider_value, 50f)
            slider.stepSize = attributes.getFloat(R.styleable.MaterialSlider_stepSize, 1f)
            attributes.recycle()
        }
    }

    fun getSlider(): Slider = slider

    fun setValue(value: Float) {
        slider.value = value.coerceIn(slider.valueFrom, slider.valueTo)
    }

    fun getValue(): Float = slider.value

    fun setValueFrom(valueFrom: Float) {
        slider.valueFrom = valueFrom
    }

    fun getValueFrom(): Float = slider.valueFrom

    fun setValueTo(valueTo: Float) {
        slider.valueTo = valueTo
    }

    fun getValueTo(): Float = slider.valueTo

    fun setStepSize(stepSize: Float) {
        slider.stepSize = stepSize
    }

    fun getStepSize(): Float = slider.stepSize

    fun setValueRange(valueFrom: Float, valueTo: Float) {
        slider.valueFrom = valueFrom
        slider.valueTo = valueTo
    }

    fun setTrackHeight(height: Int) {
        slider.trackHeight = height
    }

    fun setTrackHeightDp(heightDp: Int) {
        slider.trackHeight = DimensionUtil.dp2px(context, heightDp)
    }

    fun getTrackHeight(): Int = slider.trackHeight

    fun addOnChangeListener(listener: (value: Float, fromUser: Boolean) -> Unit) {
        onChangeListener = listener
    }

    fun removeOnChangeListener() {
        onChangeListener = null
    }

    fun addOnSliderTouchListener(listener: OnSliderTouchListener) {
        slider.addOnSliderTouchListener(listener)
    }

    fun removeOnSliderTouchListener() {
        slider.clearOnSliderTouchListeners()
    }

    override fun setEnabled(enabled: Boolean) {
        super.setEnabled(enabled)
        slider.isEnabled = enabled
    }

    override fun isEnabled(): Boolean = slider.isEnabled

    fun setSliderClickable(clickable: Boolean) {
        slider.isClickable = clickable
    }

    companion object {
        fun create(
            context: Context,
            valueFrom: Float = 0f,
            valueTo: Float = 100f,
            value: Float = 50f,
            stepSize: Float = 1f
        ): MaterialSlider {
            return MaterialSlider(context).apply {
                setValueRange(valueFrom, valueTo)
                setValue(value)
                setStepSize(stepSize)
            }
        }
    }
}
