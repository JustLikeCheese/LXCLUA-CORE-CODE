@file:SuppressLint("CustomViewStyleable")

package com.difierline.lua.material.swiperefreshlayout

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Context
import android.content.res.TypedArray
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.RadialGradient
import android.graphics.Shader
import android.graphics.drawable.GradientDrawable
import android.graphics.drawable.LayerDrawable
import android.graphics.drawable.ShapeDrawable
import android.graphics.drawable.shapes.OvalShape
import android.os.Build
import android.util.AttributeSet
import android.view.View
import android.view.animation.Animation
import androidx.appcompat.widget.AppCompatImageView
import androidx.core.content.ContextCompat
import androidx.core.view.ViewCompat
import androidx.swiperefreshlayout.R as R2
import com.difierline.lua.material.R
import com.difierline.lua.material.Material
import com.difierline.lua.utils.ColorUtil
import com.difierline.lua.utils.SizeUtil

/**
 * Private class created to work around issues with AnimationListeners being
 * called before the animation is actually complete and support shadows on older
 * platforms.
 */
class CircleImageView : AppCompatImageView {

    private var mListener: Animation.AnimationListener? = null
    private var mShadowRadius: Int = 0
    private var mBackgroundColor: Int = 0

    constructor(context: Context) : super(context) {
        init(null, 0)
    }

    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs) {
        init(attrs, 0)
    }

    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr) {
        init(attrs, defStyleAttr)
    }

    private fun init(attrs: AttributeSet?, defStyleAttr: Int) {
        val density = context.resources.displayMetrics.density
        val shadowYOffset = (density * Y_OFFSET).toInt()
        val shadowXOffset = (density * X_OFFSET).toInt()

        mShadowRadius = (density * SHADOW_RADIUS).toInt()

        // The style attribute is named SwipeRefreshLayout instead of CircleImageView because
        // CircleImageView is not part of the public api.
        @SuppressLint("CustomViewStyleable")
        val colorArray = context.obtainStyledAttributes(R2.styleable.SwipeRefreshLayout)
        mBackgroundColor = colorArray.getColor(
            R2.styleable.SwipeRefreshLayout_swipeRefreshLayoutProgressSpinnerBackgroundColor,
            DEFAULT_BACKGROUND_COLOR
        )
        colorArray.recycle()

        val gradientDrawable = GradientDrawable()
        gradientDrawable.shape = GradientDrawable.RECTANGLE
        gradientDrawable.cornerRadius = 360f
        gradientDrawable.setColor(if (Material.isMaterial3Enabled(context)) {
           ColorUtil.getColor(context as Activity, "colorSurfaceContainerLow")
         } else {
           ColorUtil.getColor(context as Activity, "colorBackground")
         })
        gradientDrawable.setStroke(SizeUtil.dp2px(context as Activity, 1), ColorUtil.getColor2(context as Activity, R.color.colorBorder))
       
        setBackground(gradientDrawable)
    }

    private fun elevationSupported(): Boolean {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP
    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec)
        if (!elevationSupported()) {
            setMeasuredDimension(measuredWidth + mShadowRadius * 2, measuredHeight + mShadowRadius * 2)
        }
    }

    fun setAnimationListener(listener: Animation.AnimationListener?) {
        mListener = listener
    }
    
    override fun onAnimationStart() {
        super.onAnimationStart()
        if (mListener != null) {
            mListener!!.onAnimationStart(animation)
        }
    }

    override fun onAnimationEnd() {
        super.onAnimationEnd()
        if (mListener != null) {
            mListener!!.onAnimationEnd(animation)
        }
    }

    override fun setBackgroundColor(color: Int) {
        if (background is ShapeDrawable) {
            (background as ShapeDrawable).paint.color = color
            mBackgroundColor = color
        }
    }

    fun getBackgroundColor(): Int {
        return mBackgroundColor
    }

    private inner class OvalShadow(private val mCircleImageView: CircleImageView, shadowRadius: Int) : OvalShape() {
        private val mShadowPaint: Paint
        private val mShadowRadius: Int

        init {
            mShadowPaint = Paint()
            mShadowRadius = shadowRadius
            updateRadialGradient(rect().width().toInt())
        }

        override fun onResize(width: Float, height: Float) {
            super.onResize(width, height)
            updateRadialGradient(width.toInt())
        }

        override fun draw(canvas: Canvas, paint: Paint) {
            val x = mCircleImageView.width / 2
            val y = mCircleImageView.height / 2
            canvas.drawCircle(x.toFloat(), y.toFloat(), x.toFloat(), mShadowPaint)
            canvas.drawCircle(x.toFloat(), y.toFloat(), x.toFloat() - mShadowRadius, paint)
        }

        private fun updateRadialGradient(diameter: Int) {
            mShadowPaint.shader = RadialGradient(
                (diameter / 2).toFloat(),
                (diameter / 2).toFloat(),
                mShadowRadius.toFloat(),
                intArrayOf(FILL_SHADOW_COLOR, Color.TRANSPARENT),
                null,
                Shader.TileMode.CLAMP
            )
        }
    }

    companion object {
        private const val DEFAULT_BACKGROUND_COLOR = 0xFFFAFAFA.toInt()
        private const val FILL_SHADOW_COLOR = 0x3D000000
        private const val KEY_SHADOW_COLOR = 0x1E000000

        // PX
        private const val X_OFFSET = 0f
        private const val Y_OFFSET = 1.75f
        private const val SHADOW_RADIUS = 3.5f
        private const val SHADOW_ELEVATION = 4
    }
}