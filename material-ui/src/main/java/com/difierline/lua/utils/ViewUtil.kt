package com.difierline.lua.utils

import android.view.View
import android.view.ViewGroup
import android.view.animation.Animation
import android.view.animation.AnimationUtils
import android.widget.FrameLayout

object ViewUtil {

    fun show(view: View) {
        view.visibility = View.VISIBLE
    }

    fun hide(view: View) {
        view.visibility = View.INVISIBLE
    }

    fun gone(view: View) {
        view.visibility = View.GONE
    }

    fun toggleVisibility(view: View) {
        view.visibility = when (view.visibility) {
            View.VISIBLE -> View.GONE
            else -> View.VISIBLE
        }
    }

    fun setVisibility(view: View, visible: Boolean) {
        view.visibility = if (visible) View.VISIBLE else View.GONE
    }

    fun setClickable(view: View, clickable: Boolean) {
        view.isClickable = clickable
    }

    fun setEnabled(view: View, enabled: Boolean) {
        view.isEnabled = enabled
    }

    fun setSelected(view: View, selected: Boolean) {
        view.isSelected = selected
    }

    fun requestFocus(view: View): Boolean {
        return view.requestFocus()
    }

    fun clearFocus(view: View) {
        view.clearFocus()
    }

    fun clearFocusWithKeyboard(view: View) {
        view.clearFocus()
        val imm = view.context.getSystemService(android.content.Context.INPUT_METHOD_SERVICE) as? android.view.inputmethod.InputMethodManager
        imm?.hideSoftInputFromWindow(view.windowToken, 0)
    }

    fun getMeasuredSize(view: View): IntArray {
        val widthMeasureSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED)
        val heightMeasureSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED)
        view.measure(widthMeasureSpec, heightMeasureSpec)
        return intArrayOf(view.measuredWidth, view.measuredHeight)
    }

    fun getSize(view: View): IntArray {
        return intArrayOf(view.width, view.height)
    }

    fun setPaddingDp(view: View, left: Int, top: Int, right: Int, bottom: Int) {
        view.setPadding(
            DimensionUtil.dp2px(view.context, left),
            DimensionUtil.dp2px(view.context, top),
            DimensionUtil.dp2px(view.context, right),
            DimensionUtil.dp2px(view.context, bottom)
        )
    }

    fun setMargin(view: View, left: Int, top: Int, right: Int, bottom: Int) {
        val params = view.layoutParams as? ViewGroup.MarginLayoutParams ?: return
        params.setMargins(left, top, right, bottom)
        view.layoutParams = params
    }

    fun setMarginDp(view: View, left: Int, top: Int, right: Int, bottom: Int) {
        val context = view.context
        val params = view.layoutParams as? ViewGroup.MarginLayoutParams ?: return
        params.setMargins(
            DimensionUtil.dp2px(context, left),
            DimensionUtil.dp2px(context, top),
            DimensionUtil.dp2px(context, right),
            DimensionUtil.dp2px(context, bottom)
        )
        view.layoutParams = params
    }

    fun setWidth(view: View, width: Int) {
        val params = view.layoutParams ?: return
        params.width = width
        view.layoutParams = params
    }

    fun setWidthDp(view: View, dpWidth: Int) {
        val params = view.layoutParams ?: return
        params.width = if (dpWidth == -1 || dpWidth == -2) dpWidth else DimensionUtil.dp2px(view.context, dpWidth)
        view.layoutParams = params
    }

    fun setHeight(view: View, height: Int) {
        val params = view.layoutParams ?: return
        params.height = height
        view.layoutParams = params
    }

    fun setHeightDp(view: View, dpHeight: Int) {
        val params = view.layoutParams ?: return
        params.height = if (dpHeight == -1 || dpHeight == -2) dpHeight else DimensionUtil.dp2px(view.context, dpHeight)
        view.layoutParams = params
    }

    fun setSize(view: View, width: Int, height: Int) {
        val params = view.layoutParams ?: return
        params.width = width
        params.height = height
        view.layoutParams = params
    }

    fun setSizeDp(view: View, dpWidth: Int, dpHeight: Int) {
        val params = view.layoutParams ?: return
        params.width = if (dpWidth == -1 || dpWidth == -2) dpWidth else DimensionUtil.dp2px(view.context, dpWidth)
        params.height = if (dpHeight == -1 || dpHeight == -2) dpHeight else DimensionUtil.dp2px(view.context, dpHeight)
        view.layoutParams = params
    }

    fun fadeIn(view: View, duration: Long = 300) {
        view.alpha = 0f
        view.visibility = View.VISIBLE
        view.animate()
            .alpha(1f)
            .setDuration(duration)
            .start()
    }

    fun fadeOut(view: View, duration: Long = 300) {
        view.animate()
            .alpha(0f)
            .setDuration(duration)
            .withEndAction {
                view.visibility = View.GONE
            }
            .start()
    }

    fun scale(view: View, scaleX: Float, scaleY: Float, duration: Long = 200) {
        view.animate()
            .scaleX(scaleX)
            .scaleY(scaleY)
            .setDuration(duration)
            .start()
    }

    fun translate(view: View, translationX: Float, translationY: Float, duration: Long = 200) {
        view.animate()
            .translationX(translationX)
            .translationY(translationY)
            .setDuration(duration)
            .start()
    }

    fun rotate(view: View, rotation: Float, duration: Long = 200) {
        view.animate()
            .rotation(rotation)
            .setDuration(duration)
            .start()
    }

    fun removeFromParent(view: View) {
        val parent = view.parent as? ViewGroup ?: return
        parent.removeView(view)
    }

    fun setBackground(view: View, backgroundDrawable: android.graphics.drawable.Drawable) {
        view.background = backgroundDrawable
    }

    fun setCircleBackground(view: View, color: Int) {
        val drawable = android.graphics.drawable.GradientDrawable().apply {
            shape = android.graphics.drawable.GradientDrawable.OVAL
            setColor(color)
        }
        view.background = drawable
    }

    fun setRoundRectBackground(view: View, color: Int, cornerRadius: Int) {
        val drawable = android.graphics.drawable.GradientDrawable().apply {
            shape = android.graphics.drawable.GradientDrawable.RECTANGLE
            setColor(color)
            setCornerRadius(DimensionUtil.dp2px(view.context, cornerRadius).toFloat())
        }
        view.background = drawable
    }

    fun setRippleBackground(view: View, rippleColor: Int? = null) {
        val colorStateList = rippleColor?.let {
            android.content.res.ColorStateList.valueOf(it)
        } ?: android.content.res.ColorStateList.valueOf(0x1F000000.toInt())
        view.background = android.graphics.drawable.RippleDrawable(colorStateList, null, null)
    }

    fun setElevation(view: View, elevation: Float) {
        view.elevation = DimensionUtil.dp2px(view.context, elevation.toInt()).toFloat()
    }

    fun clearElevation(view: View) {
        view.elevation = 0f
    }
}
