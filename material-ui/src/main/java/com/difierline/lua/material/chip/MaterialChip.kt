package com.difierline.lua.material.chip

import android.content.Context
import android.content.res.ColorStateList
import android.graphics.drawable.Drawable
import android.util.AttributeSet
import android.view.View
import com.google.android.material.chip.Chip
import com.google.android.material.chip.ChipGroup
import com.difierline.lua.material.R
import com.difierline.lua.material.Material
import com.difierline.lua.utils.ColorUtil
import com.difierline.lua.utils.DimensionUtil

class MaterialChip @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = com.google.android.material.R.attr.chipStyle
) : Chip(context, attrs, defStyleAttr) {

    private val isMaterial3Enabled = Material.isMaterial3Enabled(context)

    init {
        applyDefaultStyles()
        parseAttributes(attrs)
    }

    private fun applyDefaultStyles() {
        chipMinHeight = DimensionUtil.dp2px(context, 32).toFloat()
        chipCornerRadius = DimensionUtil.dp2px(context, if (isMaterial3Enabled) 8 else 4).toFloat()

        if (isMaterial3Enabled) {
            setChipBackgroundColorResource(R.color.chip_background_color)
            setTextColor(ColorUtil.getColor2(context as android.app.Activity, R.color.chip_text_color))
            setChipStrokeColorResource(R.color.chip_stroke_color)
            setChipStrokeWidth(DimensionUtil.dp2px(context, 1).toFloat())
        } else {
            setChipBackgroundColorResource(R.color.chip_background_material2)
            setTextColor(ColorUtil.getColor2(context as android.app.Activity, R.color.chip_text_color_material2))
        }
    }

    private fun parseAttributes(attrs: AttributeSet?) {
        attrs?.let {
            val attributes = context.obtainStyledAttributes(it, R.styleable.MaterialChip)
            text = attributes.getString(R.styleable.MaterialChip_android_text)
            isCheckable = attributes.getBoolean(R.styleable.MaterialChip_android_checkable, isCheckable)

            val chipBackgroundColor = attributes.getColorStateList(R.styleable.MaterialChip_chipBackgroundColor)
            chipBackgroundColor?.let { color -> setChipBackgroundColor(color) }

            val chipStrokeColor = attributes.getColorStateList(R.styleable.MaterialChip_chipStrokeColor)
            chipStrokeColor?.let { color -> setChipStrokeColor(color) }

            val chipStrokeWidth = attributes.getDimension(R.styleable.MaterialChip_chipStrokeWidth, 0f)
            if (chipStrokeWidth > 0) {
                setChipStrokeWidth(chipStrokeWidth)
            }

            val textColor = attributes.getColorStateList(R.styleable.MaterialChip_android_textColor)
            textColor?.let { color -> setTextColor(color) }

            val rippleColor = attributes.getColorStateList(R.styleable.MaterialChip_rippleColor)
            rippleColor?.let { color -> setRippleColor(color) }

            attributes.recycle()
        }
    }

    fun setChipStyle(type: ChipType) {
        when (type) {
            ChipType.INPUT -> {
                isCheckable = false
            }
            ChipType.CHOICE -> {
                isCheckable = true
            }
            ChipType.FILTER -> {
                isCheckable = true
            }
            ChipType.ACTION -> {
                isCheckable = false
            }
        }
    }

    fun getChipStyle(): ChipType {
        return when {
            !isCheckable && !isCloseIconVisible -> ChipType.INPUT
            isCheckable -> ChipType.CHOICE
            else -> ChipType.ACTION
        }
    }

    fun setChipTextContent(text: String?) {
        this.text = text
    }

    fun getChipTextContent(): String? = text?.toString()

    fun setChipIconDrawable(drawable: Drawable?) {
        chipIcon = drawable
    }

    fun setChipIconFromResource(resId: Int) {
        chipIcon = context.getDrawable(resId)
    }

    fun getChipIconDrawable(): Drawable? = chipIcon

    fun setChipCloseIconDrawable(drawable: Drawable?) {
        closeIcon = drawable
    }

    fun setChipCloseIconFromResource(resId: Int) {
        closeIcon = context.getDrawable(resId)
    }

    fun getChipCloseIconDrawable(): Drawable? = closeIcon

    fun setChipCloseIconVisibility(visible: Boolean) {
        isCloseIconVisible = visible
    }

    fun isChipCloseIconVisible(): Boolean = isCloseIconVisible

    fun setChipCheckedIconDrawable(drawable: Drawable?) {
        checkedIcon = drawable
    }

    fun setChipCheckedIconResource(resId: Int) {
        checkedIcon = context.getDrawable(resId)
    }

    fun getChipCheckedIconDrawable(): Drawable? = checkedIcon

    fun setChipBackground(colorStateList: ColorStateList) {
        chipBackgroundColor = colorStateList
    }

    fun setChipBackgroundColorInt(color: Int) {
        chipBackgroundColor = ColorStateList.valueOf(color)
    }

    fun getChipBackgroundColorStateList(): ColorStateList? = chipBackgroundColor

    fun setChipStroke(colorStateList: ColorStateList) {
        chipStrokeColor = colorStateList
    }

    fun setChipStrokeColorInt(color: Int) {
        chipStrokeColor = ColorStateList.valueOf(color)
    }

    fun getChipStrokeColorStateList(): ColorStateList? = chipStrokeColor

    fun setChipStrokeWidthValue(width: Float) {
        chipStrokeWidth = width
    }

    fun getChipStrokeWidthValue(): Float = chipStrokeWidth

    fun setChipCornerRadiusValue(radius: Float) {
        chipCornerRadius = radius
    }

    fun setChipCornerRadiusDp(radiusDp: Int) {
        chipCornerRadius = DimensionUtil.dp2px(context, radiusDp).toFloat()
    }

    fun getChipCornerRadiusValue(): Float = chipCornerRadius

    fun setChipRippleColor(colorStateList: ColorStateList) {
        rippleColor = colorStateList
    }

    fun setChipRippleColorInt(color: Int) {
        rippleColor = ColorStateList.valueOf(color)
    }

    fun getChipRippleColor(): ColorStateList? = rippleColor

    fun setChipIconTintColor(colorStateList: ColorStateList) {
        chipIconTint = colorStateList
    }

    fun setChipIconTintColorInt(color: Int) {
        chipIconTint = ColorStateList.valueOf(color)
    }

    fun getChipIconTintColor(): ColorStateList? = chipIconTint

    fun setChipCloseIconTintColor(colorStateList: ColorStateList) {
        closeIconTint = colorStateList
    }

    fun setChipCloseIconTintColorInt(color: Int) {
        closeIconTint = ColorStateList.valueOf(color)
    }

    fun getChipCloseIconTintColor(): ColorStateList? = closeIconTint

    fun setOnChipClickListener(listener: View.OnClickListener?) {
        setOnClickListener(listener)
    }

    fun setOnChipCloseIconClickListener(listener: View.OnClickListener?) {
        setOnCloseIconClickListener(listener)
    }

    fun setOnChipCheckedChangeListener(listener: (chip: MaterialChip, isChecked: Boolean) -> Unit) {
        super.setOnCheckedChangeListener { buttonView, isChecked ->
            listener(this, isChecked)
        }
    }

    enum class ChipType {
        INPUT,
        CHOICE,
        FILTER,
        ACTION
    }
}

class MaterialChipGroup @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = com.google.android.material.R.attr.chipGroupStyle
) : ChipGroup(context, attrs, defStyleAttr) {

    private val isMaterial3Enabled = Material.isMaterial3Enabled(context)

    init {
        applyDefaultStyles()
        parseAttributes(attrs)
    }

    private fun applyDefaultStyles() {
        if (isMaterial3Enabled) {
            isSingleSelection = false
            isSelectionRequired = false
        }
    }

    private fun parseAttributes(attrs: AttributeSet?) {
        attrs?.let {
            val attributes = context.obtainStyledAttributes(it, R.styleable.MaterialChipGroup)
            isSingleSelection = attributes.getBoolean(R.styleable.MaterialChipGroup_singleSelection, isSingleSelection)
            isSelectionRequired = attributes.getBoolean(R.styleable.MaterialChipGroup_selectionRequired, isSelectionRequired)
            val checkedChipId = attributes.getResourceId(R.styleable.MaterialChipGroup_checkedChip, View.NO_ID)
            if (checkedChipId != View.NO_ID) {
                check(checkedChipId)
            }
            attributes.recycle()
        }
    }

    fun setChipGroupSingleSelection(singleSelection: Boolean) {
        isSingleSelection = singleSelection
    }

    fun isChipGroupSingleSelection(): Boolean = isSingleSelection

    fun setChipGroupSelectionRequired(required: Boolean) {
        isSelectionRequired = required
    }

    fun isChipGroupSelectionRequired(): Boolean = isSelectionRequired

    fun getChipGroupCheckedChipId(): Int = checkedChipId

    fun getChipGroupCheckedChip(): MaterialChip? {
        val checkedId = checkedChipId
        return if (checkedId != View.NO_ID) {
            findViewById(checkedId)
        } else {
            null
        }
    }

    fun checkChip(chip: MaterialChip) {
        check(chip.id)
    }

    fun clearChipGroupCheck() {
        super.clearCheck()
    }

    fun setOnChipGroupCheckedChangeListener(listener: (group: ChipGroup?, checkedId: Int) -> Unit) {
        super.setOnCheckedChangeListener { group, checkedId ->
            listener(group, checkedId)
        }
    }

    fun addChipView(chip: MaterialChip) {
        addView(chip)
    }

    fun removeChipView(chip: MaterialChip) {
        removeView(chip)
    }

    fun removeChipViewById(id: Int) {
        val chip = findViewById<MaterialChip>(id)
        chip?.let { removeView(it) }
    }

    fun removeAllChipViews() {
        removeAllViews()
    }

    fun getAllChipViews(): List<MaterialChip> {
        val chips = mutableListOf<MaterialChip>()
        for (i in 0 until childCount) {
            val child = getChildAt(i)
            if (child is MaterialChip) {
                chips.add(child)
            }
        }
        return chips
    }

    fun getCheckedChipViews(): List<MaterialChip> {
        val checkedChips = mutableListOf<MaterialChip>()
        for (chip in getAllChipViews()) {
            if (chip.isChecked) {
                checkedChips.add(chip)
            }
        }
        return checkedChips
    }

    fun getCheckedChipViewIds(): List<Int> {
        val checkedIds = mutableListOf<Int>()
        for (chip in getAllChipViews()) {
            if (chip.isChecked) {
                checkedIds.add(chip.id)
            }
        }
        return checkedIds
    }
}
