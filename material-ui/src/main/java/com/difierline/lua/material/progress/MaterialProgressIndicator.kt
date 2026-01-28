package com.difierline.lua.material.progress

import android.content.Context
import android.util.AttributeSet
import android.view.View
import android.widget.FrameLayout
import com.google.android.material.progressindicator.CircularProgressIndicator
import com.google.android.material.progressindicator.LinearProgressIndicator
import com.difierline.lua.material.R
import com.difierline.lua.material.Material
import com.difierline.lua.utils.DimensionUtil

class MaterialCircularProgressIndicator @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : FrameLayout(context, attrs, defStyleAttr) {

    private val progressIndicator: CircularProgressIndicator

    init {
        progressIndicator = CircularProgressIndicator(context).apply {
            layoutParams = LayoutParams(
                DimensionUtil.dp2px(context, 48),
                DimensionUtil.dp2px(context, 48)
            )
            isIndeterminate = true
        }
        addView(progressIndicator)
    }

    fun setIndeterminate(indeterminate: Boolean) {
        progressIndicator.isIndeterminate = indeterminate
    }

    fun setProgress(progress: Int) {
        progressIndicator.isIndeterminate = false
        progressIndicator.progress = progress
    }

    fun getProgress(): Int = progressIndicator.progress

    fun setMaxProgress(max: Int) {
        progressIndicator.max = max
    }

    fun getMaxProgress(): Int = progressIndicator.max

    fun setIndicatorColor(color: Int) {
        progressIndicator.setIndicatorColor(color)
    }

    fun setTrackColor(color: Int) {
        progressIndicator.setTrackColor(color)
    }

    fun setIndicatorSize(size: Int) {
        val params = progressIndicator.layoutParams
        params.width = size
        params.height = size
        progressIndicator.layoutParams = params
    }

    fun setIndicatorSizeDp(sizeDp: Int) {
        setIndicatorSize(DimensionUtil.dp2px(context, sizeDp))
    }

    fun show() {
        visibility = View.VISIBLE
    }

    fun hide() {
        visibility = View.GONE
    }

    companion object {
        fun create(context: Context, indeterminate: Boolean = true, sizeDp: Int = 48): MaterialCircularProgressIndicator {
            return MaterialCircularProgressIndicator(context).apply {
                setIndeterminate(indeterminate)
                setIndicatorSizeDp(sizeDp)
            }
        }
    }
}

class MaterialLinearProgressIndicator @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : FrameLayout(context, attrs, defStyleAttr) {

    private val progressIndicator: LinearProgressIndicator

    init {
        progressIndicator = LinearProgressIndicator(context).apply {
            layoutParams = LayoutParams(
                LayoutParams.MATCH_PARENT,
                DimensionUtil.dp2px(context, 4)
            )
            isIndeterminate = true
        }
        addView(progressIndicator)
    }

    fun setIndeterminate(indeterminate: Boolean) {
        progressIndicator.isIndeterminate = indeterminate
    }

    fun setProgress(progress: Int) {
        progressIndicator.isIndeterminate = false
        progressIndicator.progress = progress
    }

    fun getProgress(): Int = progressIndicator.progress

    fun setMaxProgress(max: Int) {
        progressIndicator.max = max
    }

    fun getMaxProgress(): Int = progressIndicator.max

    fun setIndicatorColor(color: Int) {
        progressIndicator.setIndicatorColor(color)
    }

    fun setTrackColor(color: Int) {
        progressIndicator.setTrackColor(color)
    }

    fun setTrackThickness(thickness: Int) {
        progressIndicator.trackThickness = thickness
    }

    fun setTrackThicknessDp(thicknessDp: Int) {
        progressIndicator.trackThickness = DimensionUtil.dp2px(context, thicknessDp)
    }

    fun show() {
        visibility = View.VISIBLE
    }

    fun hide() {
        visibility = View.GONE
    }

    companion object {
        fun create(context: Context, indeterminate: Boolean = false, trackThicknessDp: Int = 4): MaterialLinearProgressIndicator {
            return MaterialLinearProgressIndicator(context).apply {
                setIndeterminate(indeterminate)
                setTrackThicknessDp(trackThicknessDp)
            }
        }
    }
}
