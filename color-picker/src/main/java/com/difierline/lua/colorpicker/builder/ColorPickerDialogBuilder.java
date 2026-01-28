package com.difierline.lua.colorpicker.builder;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;
import androidx.appcompat.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.GradientDrawable;
import android.text.InputFilter;
import android.text.TextWatcher;
import android.text.Editable;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.os.Handler;

import com.difierline.lua.colorpicker.ColorPickerView;
import com.difierline.lua.colorpicker.OnColorChangedListener;
import com.difierline.lua.colorpicker.OnColorSelectedListener;
import com.difierline.lua.colorpicker.R;
import com.difierline.lua.colorpicker.Utils;
import com.difierline.lua.colorpicker.renderer.ColorWheelRenderer;
import com.difierline.lua.colorpicker.slider.AlphaSlider;
import com.difierline.lua.colorpicker.slider.LightnessSlider;

import com.difierline.lua.material.textfield.MaterialTextField;

public class ColorPickerDialogBuilder {
    private MaterialAlertDialogBuilder builder;
    private LinearLayout pickerContainer;
    private ColorPickerView colorPickerView;
    private LightnessSlider lightnessSlider;
    private AlphaSlider alphaSlider;
    private LinearLayout colorPreview;
    private MaterialTextField colorEdit;
    
    private DialogInterface.OnCancelListener onCancelListener;

    private boolean isLightnessSliderEnabled = true;
    private boolean isAlphaSliderEnabled = true;
    private boolean isBorderEnabled = true;
    private boolean isColorEditEnabled = false;
    private boolean isPreviewEnabled = false;
    private int pickerCount = 1;
    private int defaultMargin = 0;
    private int defaultMarginTop = 0;
    private Integer[] initialColor = new Integer[] {null, null, null, null, null};

    private ColorPickerDialogBuilder(Context context) {
        this(context, 0);
    }

    private ColorPickerDialogBuilder(Context context, int theme) {
        defaultMargin = getDimensionAsPx(context, R.dimen.default_slider_margin);
        defaultMarginTop = getDimensionAsPx(context, R.dimen.default_margin_top);

        if (theme == 0) {
            builder = new MaterialAlertDialogBuilder(context);
        } else {
            builder = new MaterialAlertDialogBuilder(context, theme);
        }
        pickerContainer = new LinearLayout(context);
        pickerContainer.setOrientation(LinearLayout.VERTICAL);
        pickerContainer.setGravity(Gravity.CENTER_HORIZONTAL);
        pickerContainer.setPadding(defaultMargin, defaultMarginTop, defaultMargin, 0);

        LinearLayout.LayoutParams layoutParamsForColorPickerView =
                new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0);
        layoutParamsForColorPickerView.weight = 1;
        colorPickerView = new ColorPickerView(context);

        pickerContainer.addView(colorPickerView, layoutParamsForColorPickerView);

        builder.setView(pickerContainer);
    }

    public static ColorPickerDialogBuilder with(Context context) {
        return new ColorPickerDialogBuilder(context);
    }

    public static ColorPickerDialogBuilder with(Context context, int theme) {
        return new ColorPickerDialogBuilder(context, theme);
    }

    public ColorPickerDialogBuilder setTitle(String title) {
        builder.setTitle(title);
        return this;
    }

    public ColorPickerDialogBuilder setTitle(int titleId) {
        builder.setTitle(titleId);
        return this;
    }

    public ColorPickerDialogBuilder initialColor(int initialColor) {
        this.initialColor[0] = initialColor;
        return this;
    }

    public ColorPickerDialogBuilder initialColors(int[] initialColor) {
        for (int i = 0; i < initialColor.length && i < this.initialColor.length; i++) {
            this.initialColor[i] = initialColor[i];
        }
        return this;
    }

    public ColorPickerDialogBuilder wheelType(ColorPickerView.WHEEL_TYPE wheelType) {
        ColorWheelRenderer renderer = ColorWheelRendererBuilder.getRenderer(wheelType);
        colorPickerView.setRenderer(renderer);
        return this;
    }

    public ColorPickerDialogBuilder density(int density) {
        colorPickerView.setDensity(density);
        return this;
    }

    public ColorPickerDialogBuilder setOnColorChangedListener(
            OnColorChangedListener onColorChangedListener) {
        colorPickerView.addOnColorChangedListener(onColorChangedListener);
        return this;
    }

    public ColorPickerDialogBuilder setOnColorSelectedListener(
            OnColorSelectedListener onColorSelectedListener) {
        colorPickerView.addOnColorSelectedListener(onColorSelectedListener);
        return this;
    }

    public ColorPickerDialogBuilder setPositiveButton(
            CharSequence text, final ColorPickerClickListener onClickListener) {
        builder.setPositiveButton(
                text,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        positiveButtonOnClick(dialog, onClickListener);
                    }
                });
        return this;
    }

    public ColorPickerDialogBuilder setPositiveButton(
            int textId, final ColorPickerClickListener onClickListener) {
        builder.setPositiveButton(
                textId,
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        positiveButtonOnClick(dialog, onClickListener);
                    }
                });
        return this;
    }

    public ColorPickerDialogBuilder setNegativeButton(
            CharSequence text, DialogInterface.OnClickListener onClickListener) {
        builder.setNegativeButton(text, onClickListener);
        return this;
    }

    public ColorPickerDialogBuilder setNegativeButton(
            int textId, DialogInterface.OnClickListener onClickListener) {
        builder.setNegativeButton(textId, onClickListener);
        return this;
    }
    
    public ColorPickerDialogBuilder setOnCancelListener(DialogInterface.OnCancelListener listener) {
        this.onCancelListener = listener;
        return this;
    }

    public ColorPickerDialogBuilder noSliders() {
        isLightnessSliderEnabled = false;
        isAlphaSliderEnabled = false;
        return this;
    }

    public ColorPickerDialogBuilder alphaSliderOnly() {
        isLightnessSliderEnabled = false;
        isAlphaSliderEnabled = true;
        return this;
    }

    public ColorPickerDialogBuilder lightnessSliderOnly() {
        isLightnessSliderEnabled = true;
        isAlphaSliderEnabled = false;
        return this;
    }

    public ColorPickerDialogBuilder showAlphaSlider(boolean showAlpha) {
        isAlphaSliderEnabled = showAlpha;
        return this;
    }

    public ColorPickerDialogBuilder showLightnessSlider(boolean showLightness) {
        isLightnessSliderEnabled = showLightness;
        return this;
    }

    public ColorPickerDialogBuilder showBorder(boolean showBorder) {
        isBorderEnabled = showBorder;
        return this;
    }

    public ColorPickerDialogBuilder showColorEdit(boolean showEdit) {
        isColorEditEnabled = showEdit;
        return this;
    }

    public ColorPickerDialogBuilder showColorPreview(boolean showPreview) {
        isPreviewEnabled = showPreview;
        if (!showPreview) pickerCount = 1;
        return this;
    }

    public ColorPickerDialogBuilder setPickerCount(int pickerCount)
            throws IndexOutOfBoundsException {
        if (pickerCount < 1 || pickerCount > 5)
            throw new IndexOutOfBoundsException("Picker Can Only Support 1-5 Colors");
        this.pickerCount = pickerCount;
        if (this.pickerCount > 1) this.isPreviewEnabled = true;
        return this;
    }

    public AlertDialog build() {
        Context context = builder.getContext();
        colorPickerView.setInitialColors(initialColor, getStartOffset(initialColor));
        colorPickerView.setShowBorder(isBorderEnabled);

        if (isLightnessSliderEnabled) {
            LinearLayout.LayoutParams layoutParamsForLightnessBar =
                    new LinearLayout.LayoutParams(
                            ViewGroup.LayoutParams.MATCH_PARENT,
                            getDimensionAsPx(context, R.dimen.default_slider_height));
            lightnessSlider = new LightnessSlider(context);
            lightnessSlider.setLayoutParams(layoutParamsForLightnessBar);
            pickerContainer.addView(lightnessSlider);
            colorPickerView.setLightnessSlider(lightnessSlider);
            lightnessSlider.setColor(getStartColor(initialColor));
            lightnessSlider.setShowBorder(isBorderEnabled);
        }
        if (isAlphaSliderEnabled) {
            LinearLayout.LayoutParams layoutParamsForAlphaBar =
                    new LinearLayout.LayoutParams(
                            ViewGroup.LayoutParams.MATCH_PARENT,
                            getDimensionAsPx(context, R.dimen.default_slider_height));
            alphaSlider = new AlphaSlider(context);
            alphaSlider.setLayoutParams(layoutParamsForAlphaBar);
            pickerContainer.addView(alphaSlider);
            colorPickerView.setAlphaSlider(alphaSlider);
            alphaSlider.setColor(getStartColor(initialColor));
            alphaSlider.setShowBorder(isBorderEnabled);
        }

        // ===== 本次修改开始 =====
        // 若两者均启用，则用横向 LinearLayout 包裹 colorPreview 与 colorEdit
        if (isColorEditEnabled && isPreviewEnabled) {
            // 横向容器
            LinearLayout horizontalRow = new LinearLayout(context);
            horizontalRow.setOrientation(LinearLayout.HORIZONTAL);
            horizontalRow.setGravity(Gravity.CENTER);
            LinearLayout.LayoutParams rowParams =
                    new LinearLayout.LayoutParams(
                            ViewGroup.LayoutParams.MATCH_PARENT,
                            ViewGroup.LayoutParams.WRAP_CONTENT);
            rowParams.topMargin = defaultMarginTop;
            horizontalRow.setLayoutParams(rowParams);

            // 左侧 colorPreview
            colorPreview = (LinearLayout) View.inflate(context, R.layout.color_preview, null);
            colorPreview.setVisibility(View.GONE);
            LinearLayout.LayoutParams previewParams =
                    new LinearLayout.LayoutParams(
                            ViewGroup.LayoutParams.WRAP_CONTENT,
                            ViewGroup.LayoutParams.WRAP_CONTENT);
            previewParams.gravity = Gravity.CENTER_VERTICAL;
            horizontalRow.addView(colorPreview, previewParams);

            // 右侧 colorEdit
            colorEdit = new MaterialTextField(context);
            colorEdit.setHint("Hex color");
            colorEdit.setSingleLine(true);
            LinearLayout.LayoutParams editParams =
                    new LinearLayout.LayoutParams(
                            ViewGroup.LayoutParams.WRAP_CONTENT,
                            ViewGroup.LayoutParams.WRAP_CONTENT);
            editParams.gravity = Gravity.CENTER_VERTICAL;
            editParams.width = LinearLayout.LayoutParams.WRAP_CONTENT;
            editParams.leftMargin =
                    (int) (12 * context.getResources().getDisplayMetrics().density + 0.5f); // 添加左边距
            horizontalRow.addView(colorEdit, editParams);

            pickerContainer.addView(horizontalRow);

            // 初始化 colorPreview 内部子项
            if (initialColor.length == 0) {
                ImageView colorImage =
                        (ImageView) View.inflate(context, R.layout.color_selector, null);

            } else {
                for (int i = 0; i < initialColor.length && i < this.pickerCount; i++) {
                    if (initialColor[i] == null) break;
                    LinearLayout colorLayout =
                            (LinearLayout) View.inflate(context, R.layout.color_selector, null);
                    ImageView colorImage = (ImageView) colorLayout.findViewById(R.id.image_preview);
                    //colorImage.setImageDrawable(new ColorDrawable(initialColor[i]));
                    
                    GradientDrawable colorDrawable = new GradientDrawable();
        colorDrawable.setShape(GradientDrawable.OVAL);
        colorDrawable.setColor(initialColor[i]);
        colorImage.setImageDrawable(colorDrawable);
        
                    colorPreview.addView(colorLayout);
                }
            }
            colorPreview.setVisibility(View.VISIBLE);
            colorPickerView.setColorPreview(colorPreview, getStartOffset(initialColor));

            // 设置 colorEdit 初始值 & 监听
            String initialHex =
                    Utils.getHexString(getStartColor(initialColor), isAlphaSliderEnabled);
            colorEdit.setText(initialHex);
            colorEdit.addTextChangedListener(
                    new TextWatcher() {
                        private String lastValidHex = initialHex.replace("#", "");
                        private final Handler handler = new Handler();
                        private Runnable updateTask;

                        @Override
                        public void beforeTextChanged(
                                CharSequence s, int start, int count, int after) {}

                        @Override
                        public void onTextChanged(
                                CharSequence s, int start, int before, int count) {}

                        @Override
                        public void afterTextChanged(Editable s) {
                            handler.removeCallbacks(updateTask);
                            updateTask =
                                    new Runnable() {
                                        @Override
                                        public void run() {
                                            String input =
                                                    s.toString()
                                                            .replace("#", "")
                                                            .trim()
                                                            .toUpperCase();
                                            if (input.isEmpty()) return;
                                            if (input.equals(lastValidHex)) return;
                                            int requiredLength = isAlphaSliderEnabled ? 8 : 6;
                                            if (input.length() != requiredLength
                                                    || !input.matches("[0-9A-F]+")) return;
                                            try {
                                                int color = Color.parseColor("#" + input);
                                                lastValidHex = input;
                                                colorPickerView.setColor(color, true);
                                            } catch (IllegalArgumentException ignored) {
                                            }
                                        }
                                    };
                            handler.postDelayed(updateTask, 300);
                        }
                    });
            colorPickerView.setColorEdit(colorEdit.getEditText());

        } else {
            // 以下保持旧逻辑不变：单独添加 colorEdit 或 colorPreview
            if (isColorEditEnabled) {
                LinearLayout.LayoutParams layoutParamsForColorEdit =
                        new LinearLayout.LayoutParams(
                                ViewGroup.LayoutParams.MATCH_PARENT,
                                ViewGroup.LayoutParams.WRAP_CONTENT);
                layoutParamsForColorEdit.topMargin = defaultMarginTop;

                colorEdit = new MaterialTextField(context);
                colorEdit.setLayoutParams(layoutParamsForColorEdit);
                colorEdit.setHint("Hex color");
                colorEdit.setSingleLine(true);
                String initialHex =
                        Utils.getHexString(getStartColor(initialColor), isAlphaSliderEnabled);
                colorEdit.setText(initialHex);

                colorEdit.addTextChangedListener(
                        new TextWatcher() {
                            private String lastValidHex = initialHex.replace("#", "");
                            private final Handler handler = new Handler();
                            private Runnable updateTask;

                            @Override
                            public void beforeTextChanged(
                                    CharSequence s, int start, int count, int after) {}

                            @Override
                            public void onTextChanged(
                                    CharSequence s, int start, int before, int count) {}

                            @Override
                            public void afterTextChanged(Editable s) {
                                handler.removeCallbacks(updateTask);
                                updateTask =
                                        new Runnable() {
                                            @Override
                                            public void run() {
                                                String input =
                                                        s.toString()
                                                                .replace("#", "")
                                                                .trim()
                                                                .toUpperCase();
                                                if (input.isEmpty()) return;
                                                if (input.equals(lastValidHex)) return;
                                                int requiredLength = isAlphaSliderEnabled ? 8 : 6;
                                                if (input.length() != requiredLength
                                                        || !input.matches("[0-9A-F]+")) return;
                                                try {
                                                    int color = Color.parseColor("#" + input);
                                                    lastValidHex = input;
                                                    colorPickerView.setColor(color, true);
                                                } catch (IllegalArgumentException ignored) {
                                                }
                                            }
                                        };
                                handler.postDelayed(updateTask, 300);
                            }
                        });
                pickerContainer.addView(colorEdit);
                colorPickerView.setColorEdit(colorEdit.getEditText());
            }

            if (isPreviewEnabled) {
                colorPreview = (LinearLayout) View.inflate(context, R.layout.color_preview, null);
                colorPreview.setVisibility(View.GONE);
                pickerContainer.addView(colorPreview);

                if (initialColor.length == 0) {
                    ImageView colorImage =
                            (ImageView) View.inflate(context, R.layout.color_selector, null);
                } else {
                    for (int i = 0; i < initialColor.length && i < this.pickerCount; i++) {
                        if (initialColor[i] == null) break;
                        LinearLayout colorLayout =
                                (LinearLayout) View.inflate(context, R.layout.color_selector, null);
                        ImageView colorImage =
                                (ImageView) colorLayout.findViewById(R.id.image_preview);
                       // colorImage.setImageDrawable(new ColorDrawable(initialColor[i]));
                        
                        GradientDrawable colorDrawable = new GradientDrawable();
        colorDrawable.setShape(GradientDrawable.OVAL);
        colorDrawable.setColor(initialColor[i]);
        colorImage.setImageDrawable(colorDrawable);
        
                        colorPreview.addView(colorLayout);
                    }
                }
                colorPreview.setVisibility(View.VISIBLE);
                colorPickerView.setColorPreview(colorPreview, getStartOffset(initialColor));
            }
        }
        // ===== 本次修改结束 =====
        AlertDialog dialog = builder.create();
        if (onCancelListener != null) {
            dialog.setOnCancelListener(onCancelListener);
        }
        
        return dialog;
    }

    private Integer getStartOffset(Integer[] colors) {
        Integer start = 0;
        for (int i = 0; i < colors.length; i++) {
            if (colors[i] == null) {
                return start;
            }
            start = (i + 1) / 2;
        }
        return start;
    }

    private int getStartColor(Integer[] colors) {
        Integer startColor = getStartOffset(colors);
        return startColor == null ? Color.WHITE : colors[startColor];
    }

    private static int getDimensionAsPx(Context context, int rid) {
        return (int) (context.getResources().getDimension(rid) + .5f);
    }

    private void positiveButtonOnClick(
            DialogInterface dialog, ColorPickerClickListener onClickListener) {
        int selectedColor = colorPickerView.getSelectedColor();
        Integer[] allColors = colorPickerView.getAllColors();
        onClickListener.onClick(dialog, selectedColor, allColors);
    }
}
