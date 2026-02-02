package com.difierline.lua.material.bottomsheet

import android.content.Context
import android.util.AttributeSet
import android.graphics.drawable.Drawable
import android.view.LayoutInflater
import android.view.View
import android.widget.FrameLayout
import androidx.annotation.NonNull
import androidx.core.content.ContextCompat
import com.google.android.material.bottomsheet.BottomSheetBehavior
import com.google.android.material.bottomsheet.BottomSheetDialog
import com.difierline.lua.material.R
import com.difierline.lua.utils.DimensionUtil

class MaterialBottomSheet @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : FrameLayout(context, attrs, defStyleAttr) {

    private val bottomSheetDialog: BottomSheetDialog
    private val bottomSheetView: View
    private val behavior: BottomSheetBehavior<View>
    private var onStateChangedListener: ((state: Int) -> Unit)? = null
    private var onDismissListener: (() -> Unit)? = null

    init {
        bottomSheetDialog = BottomSheetDialog(context, defStyleAttr)
        bottomSheetView = LayoutInflater.from(context).inflate(R.layout.bottom_sheet, this, false)
        behavior = BottomSheetBehavior.from(bottomSheetView as View)

        setupDefaultStyle()
        setupListeners()

        bottomSheetDialog.setContentView(bottomSheetView)
        addView(bottomSheetView)
    }

    private fun setupDefaultStyle() {
        val defaultPeekHeight = DimensionUtil.dp2px(context, 200)
        behavior.state = BottomSheetBehavior.STATE_HIDDEN
        behavior.skipCollapsed = true
        behavior.peekHeight = defaultPeekHeight
        behavior.isHideable = true
        bottomSheetView.setBackgroundColor(ContextCompat.getColor(context, R.color.bottom_sheet_background))
    }

    private fun setupListeners() {
        behavior.addBottomSheetCallback(object : BottomSheetBehavior.BottomSheetCallback() {
            override fun onStateChanged(@NonNull bottomSheet: View, newState: Int) {
                onStateChangedListener?.invoke(newState)
                if (newState == BottomSheetBehavior.STATE_HIDDEN) {
                    onDismissListener?.invoke()
                }
            }

            override fun onSlide(@NonNull bottomSheet: View, slideOffset: Float) {}
        })

        bottomSheetDialog.setOnDismissListener {
            onDismissListener?.invoke()
        }
    }

    fun getDialog(): BottomSheetDialog = bottomSheetDialog

    fun getBehavior(): BottomSheetBehavior<View> = behavior

    fun getContentView(): View = bottomSheetView

    fun setPeekHeight(peekHeight: Int) {
        behavior.peekHeight = peekHeight
    }

    fun setPeekHeightDp(peekHeightDp: Int) {
        behavior.peekHeight = DimensionUtil.dp2px(context, peekHeightDp)
    }

    fun getPeekHeight(): Int = behavior.peekHeight

    fun setSkipCollapsed(skipCollapsed: Boolean) {
        behavior.skipCollapsed = skipCollapsed
    }

    fun setHideable(hideable: Boolean) {
        behavior.isHideable = hideable
    }

    fun setSheetState(state: Int) {
        behavior.state = state
    }

    fun getSheetState(): Int = behavior.state

    fun expand() {
        behavior.state = BottomSheetBehavior.STATE_EXPANDED
    }

    fun halfExpand() {
        behavior.state = BottomSheetBehavior.STATE_HALF_EXPANDED
    }

    fun collapse() {
        behavior.state = BottomSheetBehavior.STATE_COLLAPSED
    }

    fun hideSheet() {
        behavior.state = BottomSheetBehavior.STATE_HIDDEN
    }

    fun show() {
        if (behavior.state == BottomSheetBehavior.STATE_HIDDEN) {
            behavior.state = BottomSheetBehavior.STATE_COLLAPSED
        }
        bottomSheetDialog.show()
    }

    fun dismiss() {
        bottomSheetDialog.dismiss()
    }

    fun cancel() {
        bottomSheetDialog.cancel()
    }

    fun setSheetBackgroundColor(color: Int) {
        bottomSheetView.setBackgroundColor(color)
    }

    fun setContentView(view: View) {
        val contentContainer = bottomSheetView.findViewById<FrameLayout>(R.id.bottom_sheet_content)
        contentContainer.removeAllViews()
        contentContainer.addView(view)
    }

    fun getContentContainer(): FrameLayout {
        return bottomSheetView.findViewById(R.id.bottom_sheet_content)
    }

    fun setOnStateChangedListener(listener: (state: Int) -> Unit) {
        onStateChangedListener = listener
    }

    fun setOnDismissListener(listener: () -> Unit) {
        onDismissListener = listener
    }

    fun setOnCancelListener(listener: () -> Unit) {
        bottomSheetDialog.setOnCancelListener { listener() }
    }

    fun setSheetCancelable(cancelable: Boolean) {
        bottomSheetDialog.setCancelable(cancelable)
    }

    fun isSheetCancelable(): Boolean = true

    fun setCanceledOnTouchOutside(canceledOnTouchOutside: Boolean) {
        bottomSheetDialog.setCanceledOnTouchOutside(canceledOnTouchOutside)
    }

    fun getBottomSheetView(): View = bottomSheetView

    companion object {
        fun showListBottomSheet(
            context: Context,
            title: String?,
            items: Array<String>,
            onItemClick: (which: Int) -> Unit
        ): MaterialBottomSheet {
            val bottomSheet = create(context)

            val listView = android.widget.ListView(context).apply {
                adapter = android.widget.ArrayAdapter(context, android.R.layout.simple_list_item_1, items)
                setOnItemClickListener { _, _, position, _ ->
                    onItemClick(position)
                    bottomSheet.dismiss()
                }
            }

            val contentContainer = bottomSheet.getContentContainer()
            if (title != null) {
                val titleView = android.widget.TextView(context).apply {
                    text = title
                    textSize = 18f
                    setTextColor(ContextCompat.getColor(context, android.R.color.black))
                    setPadding(
                        DimensionUtil.dp2px(context, 16),
                        DimensionUtil.dp2px(context, 16),
                        DimensionUtil.dp2px(context, 16),
                        DimensionUtil.dp2px(context, 16)
                    )
                }
                contentContainer.addView(titleView)
            }

            val params = LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT)
            listView.layoutParams = params
            contentContainer.addView(listView)

            bottomSheet.show()
            return bottomSheet
        }

        fun showCustomViewBottomSheet(
            context: Context,
            view: View,
            peekHeightDp: Int = 200
        ): MaterialBottomSheet {
            val bottomSheet = create(context)
            bottomSheet.setPeekHeightDp(peekHeightDp)
            bottomSheet.setContentView(view)
            bottomSheet.show()
            return bottomSheet
        }

        fun create(context: Context): MaterialBottomSheet {
            return MaterialBottomSheet(context)
        }
    }
}

object BottomSheetState {
    const val STATE_DRAGGING = BottomSheetBehavior.STATE_DRAGGING
    const val STATE_SETTLING = BottomSheetBehavior.STATE_SETTLING
    const val STATE_EXPANDED = BottomSheetBehavior.STATE_EXPANDED
    const val STATE_COLLAPSED = BottomSheetBehavior.STATE_COLLAPSED
    const val STATE_HIDDEN = BottomSheetBehavior.STATE_HIDDEN
    const val STATE_HALF_EXPANDED = BottomSheetBehavior.STATE_HALF_EXPANDED
}
