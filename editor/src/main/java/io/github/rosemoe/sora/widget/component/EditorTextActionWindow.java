/*
 *    sora-editor - the awesome code editor for Android
 *    https://github.com/Rosemoe/sora-editor
 *    Copyright (C) 2020-2024  Rosemoe
 *
 *     This library is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU Lesser General Public
 *     License as published by the Free Software Foundation; either
 *     version 2.1 of the License, or (at your option) any later version.
 *
 *     This library is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *     Lesser General Public License for more details.
 *
 *     You should have received a copy of the GNU Lesser General Public
 *     License along with this library; if not, write to the Free Software
 *     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *     USA
 *
 *     Please contact Rosemoe by email 2073412493@qq.com if you need
 *     additional information or have any questions
 */
package io.github.rosemoe.sora.widget.component;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.RectF;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.GradientDrawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Gravity;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ImageView;
import android.widget.HorizontalScrollView;
import android.util.TypedValue;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import io.github.rosemoe.sora.R;
import io.github.rosemoe.sora.event.ColorSchemeUpdateEvent;
import io.github.rosemoe.sora.event.DragSelectStopEvent;
import io.github.rosemoe.sora.event.EditorFocusChangeEvent;
import io.github.rosemoe.sora.event.EditorReleaseEvent;
import io.github.rosemoe.sora.event.EventManager;
import io.github.rosemoe.sora.event.HandleStateChangeEvent;
import io.github.rosemoe.sora.event.InterceptTarget;
import io.github.rosemoe.sora.event.LongPressEvent;
import io.github.rosemoe.sora.event.ScrollEvent;
import io.github.rosemoe.sora.event.SelectionChangeEvent;
import io.github.rosemoe.sora.widget.CodeEditor;
import io.github.rosemoe.sora.widget.EditorTouchEventHandler;
import io.github.rosemoe.sora.widget.base.EditorPopupWindow;
import io.github.rosemoe.sora.widget.schemes.EditorColorScheme;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This window will show when selecting text to present text actions.
 *
 * @author Rosemoe
 */
public class EditorTextActionWindow extends EditorPopupWindow implements View.OnClickListener, EditorBuiltinComponent {
    private final static long DELAY = 200;
    private final static long CHECK_FOR_DISMISS_INTERVAL = 100;
    private final CodeEditor editor;
    private final ImageButton selectAllBtn;
    private final ImageButton pasteBtn;
    private final ImageButton copyBtn;
    private final ImageButton cutBtn;
    private final ImageButton longSelectBtn;
    private final View rootView;
    private final EditorTouchEventHandler handler;
    private final EventManager eventManager;
    private long lastScroll;
    private int lastPosition;
    private int lastCause;
    private boolean enabled = true;
    
    // 单例实例映射，使用HashMap
    private static final Map<CodeEditor, EditorTextActionWindow> instances = new HashMap<>();
    
    // 自定义按钮列表
    private final List<CustomButton> customEditableButtons = new ArrayList<>();
    private final List<CustomButton> customNonEditableButtons = new ArrayList<>();
    
    // 标记是否已清空按钮
    private boolean buttonsCleared = false;
    
    // 控制搜索按钮是否显示
    private boolean searchButtonVisible = false;

    /**
     * Create a panel for the given editor
     *
     * @param editor Target editor
     */
    public EditorTextActionWindow(CodeEditor editor) {
        super(editor, FEATURE_SHOW_OUTSIDE_VIEW_ALLOWED);
        this.editor = editor;
        handler = editor.getEventHandler();
        eventManager = editor.createSubEventManager();

        // Since popup window does provide decor view, we have to pass null to this method
        @SuppressLint("InflateParams")
        View root = this.rootView = LayoutInflater.from(editor.getContext()).inflate(R.layout.text_compose_panel, null);
        selectAllBtn = root.findViewById(R.id.panel_btn_select_all);
        cutBtn = root.findViewById(R.id.panel_btn_cut);
        copyBtn = root.findViewById(R.id.panel_btn_copy);
        longSelectBtn = root.findViewById(R.id.panel_btn_long_select);
        pasteBtn = root.findViewById(R.id.panel_btn_paste);

        selectAllBtn.setOnClickListener(this);
        cutBtn.setOnClickListener(this);
        copyBtn.setOnClickListener(this);
        pasteBtn.setOnClickListener(this);
        longSelectBtn.setOnClickListener(this);

        applyColorScheme();
        setContentView(root);
        setSize(0, (int) (this.editor.getDpUnit() * 48));
        getPopup().setAnimationStyle(R.style.text_action_popup_animation);

        subscribeEvents();
    }
    
    /**
     * 获取单例实例
     *
     * @param editor Target editor
     * @return EditorTextActionWindow 实例
     */
    public static EditorTextActionWindow getInstance(CodeEditor editor) {
        EditorTextActionWindow instance = instances.get(editor);
        if (instance == null) {
            instance = new EditorTextActionWindow(editor);
            instances.put(editor, instance);
        }
        return instance;
    }

    /**
     * 清除指定编辑器的实例
     *
     * @param editor Target editor
     */
    public static void clearInstance(CodeEditor editor) {
        EditorTextActionWindow instance = instances.get(editor);
        if (instance != null) {
            instance.setEnabled(false);
            instances.remove(editor);
        }
    }

    /** 清除所有实例 */
    public static void clearAllInstances() {
        for (EditorTextActionWindow instance : instances.values()) {
            instance.setEnabled(false);
        }
        instances.clear();
    }

    /**
     * 设置搜索按钮是否可见
     *
     * @param visible true表示显示搜索按钮，false表示隐藏搜索按钮
     */
    public void setSearchButtonVisible(boolean visible) {
        if (searchButtonVisible != visible) {
            searchButtonVisible = visible;
            // 如果窗口正在显示，立即更新按钮状态
            if (isShowing()) {
                updateBtnState();
            }
        }
    }

    /**
     * 获取搜索按钮的可见状态
     *
     * @return true表示搜索按钮可见，false表示隐藏
     */
    public boolean isSearchButtonVisible() {
        return searchButtonVisible;
    }

    /**
     * 清空所有按钮
     * 
     * 调用时机说明：
     * - 在addBtn之前调用：只清空内置按钮，后续addBtn添加的按钮不受影响
     * - 在addBtn之后调用：清空所有按钮（包括内置按钮和自定义按钮）
     */
    public void clearButtons() {
        // 清空自定义按钮列表
        customEditableButtons.clear();
        customNonEditableButtons.clear();
        
        // 设置清空标志
        buttonsCleared = true;
        
        // 更新按钮状态以反映清空结果
        if (isShowing()) {
            updateBtnState();
        }
    }

    /**
     * 重置按钮状态，恢复默认的内置按钮
     */
    public void resetButtons() {
        buttonsCleared = false;
        if (isShowing()) {
            updateBtnState();
        }
    }

    /**
     * 添加自定义按钮（使用资源ID）
     *
     * @param isEditable 如果为true，则在编辑器可编辑状态下显示；如果为false，则在不可编辑状态下显示
     * @param text 按钮名称（用于内容描述）
     * @param icon 按钮图标资源ID
     * @param function 点击按钮时执行的功能
     */
    public void addBtn(boolean isEditable, String text, int icon, Runnable function) {
        int id = View.generateViewId(); // 生成唯一ID
        CustomButton customButton = new CustomButton(id, text, icon, function);

        if (isEditable) {
            customEditableButtons.add(customButton);
        } else {
            customNonEditableButtons.add(customButton);
        }
    }

    /**
     * 添加自定义按钮（使用Drawable）
     *
     * @param isEditable 如果为true，则在编辑器可编辑状态下显示；如果为false，则在不可编辑状态下显示
     * @param text 按钮名称（用于内容描述）
     * @param icon 按钮图标Drawable
     * @param function 点击按钮时执行的功能
     */
    public void addBtn(boolean isEditable, String text, Drawable icon, Runnable function) {
        int id = View.generateViewId(); // 生成唯一ID
        CustomButton customButton = new CustomButton(id, text, icon, function);

        if (isEditable) {
            customEditableButtons.add(customButton);
        } else {
            customNonEditableButtons.add(customButton);
        }
    }

    protected void applyColorFilter(ImageButton btn, int color) {
        var drawable = btn.getDrawable();
        if (drawable == null) {
            return;
        }
        btn.setColorFilter(new PorterDuffColorFilter(color, PorterDuff.Mode.SRC_ATOP));
    }

    protected void applyColorScheme() {
        GradientDrawable gd = new GradientDrawable();
        gd.setCornerRadius(5 * editor.getDpUnit());
        gd.setColor(editor.getColorScheme().getColor(EditorColorScheme.TEXT_ACTION_WINDOW_BACKGROUND));
        int strokeColor = editor.getColorScheme().getColor(EditorColorScheme.TEXT_ACTION_WINDOW_STROKE_COLOR);
        gd.setStroke(3, strokeColor);
        rootView.setBackground(gd);
        int color = editor.getColorScheme().getColor(EditorColorScheme.TEXT_ACTION_WINDOW_ICON_COLOR);
        applyColorFilter(selectAllBtn, color);
        applyColorFilter(cutBtn, color);
        applyColorFilter(copyBtn, color);
        applyColorFilter(pasteBtn, color);
        applyColorFilter(longSelectBtn, color);
    }

    protected void subscribeEvents() {
        eventManager.subscribeAlways(SelectionChangeEvent.class, this::onSelectionChange);
        eventManager.subscribeAlways(ScrollEvent.class, this::onEditorScroll);
        eventManager.subscribeAlways(HandleStateChangeEvent.class, this::onHandleStateChange);
        eventManager.subscribeAlways(LongPressEvent.class, this::onEditorLongPress);
        eventManager.subscribeAlways(EditorFocusChangeEvent.class, this::onEditorFocusChange);
        eventManager.subscribeAlways(EditorReleaseEvent.class, this::onEditorRelease);
        eventManager.subscribeAlways(ColorSchemeUpdateEvent.class, this::onEditorColorChange);
        eventManager.subscribeAlways(DragSelectStopEvent.class, this::onDragSelectingStop);
    }

    protected void onEditorColorChange(@NonNull ColorSchemeUpdateEvent event) {
        applyColorScheme();
    }

    protected void onEditorFocusChange(@NonNull EditorFocusChangeEvent event) {
        if (!event.isGainFocus()) {
            dismiss();
        }
    }

    protected void onDragSelectingStop(@NonNull DragSelectStopEvent event) {
        displayWindow();
    }

    protected void onEditorRelease(@NonNull EditorReleaseEvent event) {
        setEnabled(false);
        // 编辑器释放时清除实例
        clearInstance(editor);
    }

    protected void onEditorLongPress(@NonNull LongPressEvent event) {
        if (editor.getCursor().isSelected() && lastCause == SelectionChangeEvent.CAUSE_SEARCH) {
            var idx = event.getIndex();
            if (idx >= editor.getCursor().getLeft() && idx <= editor.getCursor().getRight()) {
                lastCause = 0;
                displayWindow();
            }
            event.intercept(InterceptTarget.TARGET_EDITOR);
        }
    }

    protected void onEditorScroll(@NonNull ScrollEvent event) {
        var last = lastScroll;
        lastScroll = System.currentTimeMillis();
        if (lastScroll - last < DELAY && lastCause != SelectionChangeEvent.CAUSE_SEARCH) {
            postDisplay();
        }
    }

    protected void onHandleStateChange(@NonNull HandleStateChangeEvent event) {
        if (event.isHeld()) {
            postDisplay();
        }
        if (!event.getEditor().getCursor().isSelected()
                && event.getHandleType() == HandleStateChangeEvent.HANDLE_TYPE_INSERT
                && !event.isHeld()) {
            displayWindow();
            // Also, post to hide the window on handle disappearance
            editor.postDelayedInLifecycle(new Runnable() {
                @Override
                public void run() {
                    if (!editor.getEventHandler().shouldDrawInsertHandle()
                            && !editor.getCursor().isSelected()) {
                        dismiss();
                    } else if (!editor.getCursor().isSelected()) {
                        editor.postDelayedInLifecycle(this, CHECK_FOR_DISMISS_INTERVAL);
                    }
                }
            }, CHECK_FOR_DISMISS_INTERVAL);
        }
    }

    protected void onSelectionChange(@NonNull SelectionChangeEvent event) {
        if (handler.hasAnyHeldHandle() || event.getCause() == SelectionChangeEvent.CAUSE_DEAD_KEYS) {
            return;
        }
        if (handler.isDragSelecting()) {
            dismiss();
            return;
        }
        lastCause = event.getCause();
        if (event.isSelected() || event.getCause() == SelectionChangeEvent.CAUSE_LONG_PRESS && editor.getText().length() == 0) {
            // Always post show. See #193
            if (event.getCause() != SelectionChangeEvent.CAUSE_SEARCH) {
                editor.postInLifecycle(this::displayWindow);
            } else {
                dismiss();
            }
            lastPosition = -1;
        } else {
            var show = false;
            if (event.getCause() == SelectionChangeEvent.CAUSE_TAP && event.getLeft().index == lastPosition && !isShowing() && !editor.getText().isInBatchEdit() && editor.isEditable()) {
                editor.postInLifecycle(this::displayWindow);
                show = true;
            } else {
                dismiss();
            }
            if (event.getCause() == SelectionChangeEvent.CAUSE_TAP && !show) {
                lastPosition = event.getLeft().index;
            } else {
                lastPosition = -1;
            }
        }
    }

    @Override
    public boolean isEnabled() {
        return enabled;
    }

    @Override
    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
        eventManager.setEnabled(enabled);
        if (!enabled) {
            dismiss();
        }
    }

    /**
     * Get the view root of the panel.
     * <p>
     * Root view is {@link android.widget.LinearLayout}
     * Inside is a {@link android.widget.HorizontalScrollView}
     *
     * @see R.id#panel_root
     * @see R.id#panel_hv
     * @see R.id#panel_btn_select_all
     * @see R.id#panel_btn_copy
     * @see R.id#panel_btn_cut
     * @see R.id#panel_btn_paste
     */
    public ViewGroup getView() {
        return (ViewGroup) getPopup().getContentView();
    }

    private void postDisplay() {
        if (!isShowing()) {
            return;
        }
        dismiss();
        if (!editor.getCursor().isSelected()) {
            return;
        }
        editor.postDelayedInLifecycle(new Runnable() {
            @Override
            public void run() {
                if (!handler.hasAnyHeldHandle() && !editor.getSnippetController().isInSnippet() && System.currentTimeMillis() - lastScroll > DELAY
                        && editor.getScroller().isFinished()) {
                    displayWindow();
                } else {
                    editor.postDelayedInLifecycle(this, DELAY);
                }
            }
        }, DELAY);
    }

    private int selectTop(@NonNull RectF rect) {
        var rowHeight = editor.getRowHeight();
        if (rect.top - rowHeight * 3 / 2F > getHeight()) {
            return (int) (rect.top - rowHeight * 3 / 2 - getHeight());
        } else {
            return (int) (rect.bottom + rowHeight / 2);
        }
    }

    public void displayWindow() {
        updateBtnState();
        int top;
        var cursor = editor.getCursor();
        if (cursor.isSelected()) {
            var leftRect = editor.getLeftHandleDescriptor().position;
            var rightRect = editor.getRightHandleDescriptor().position;
            var top1 = selectTop(leftRect);
            var top2 = selectTop(rightRect);
            top = Math.min(top1, top2);
        } else {
            top = selectTop(editor.getInsertHandleDescriptor().position);
        }
        top = Math.max(0, Math.min(top, editor.getHeight() - getHeight() - 5));
        float handleLeftX = editor.getOffset(editor.getCursor().getLeftLine(), editor.getCursor().getLeftColumn());
        float handleRightX = editor.getOffset(editor.getCursor().getRightLine(), editor.getCursor().getRightColumn());
        int panelX = (int) ((handleLeftX + handleRightX) / 2f - rootView.getMeasuredWidth() / 2f);
        setLocationAbsolutely(panelX, top);
        show();
    }

    /**
     * Update the state of paste button
     */
    private void updateBtnState() {
        pasteBtn.setEnabled(editor.hasClip());
        copyBtn.setVisibility(editor.getCursor().isSelected() ? View.VISIBLE : View.GONE);
        pasteBtn.setVisibility(editor.isEditable() ? View.VISIBLE : View.GONE);
        cutBtn.setVisibility((editor.getCursor().isSelected() && editor.isEditable()) ? View.VISIBLE : View.GONE);
        longSelectBtn.setVisibility((!editor.getCursor().isSelected() && editor.isEditable()) ? View.VISIBLE : View.GONE);
        
        // 处理内置按钮的显示
        if (buttonsCleared) {
            selectAllBtn.setVisibility(View.GONE);
            pasteBtn.setVisibility(View.GONE);
            copyBtn.setVisibility(View.GONE);
            cutBtn.setVisibility(View.GONE);
            longSelectBtn.setVisibility(View.GONE);
        } else {
            selectAllBtn.setVisibility(View.VISIBLE);
        }
        
        // 添加自定义按钮
        LinearLayout buttonContainer = null;
        
        // 尝试获取内部LinearLayout
        HorizontalScrollView hv = rootView.findViewById(R.id.panel_hv);
        if (hv != null && hv.getChildCount() > 0) {
            View child = hv.getChildAt(0);
            if (child instanceof LinearLayout) {
                buttonContainer = (LinearLayout) child;
            }
        }
        
        // 如果找不到，使用panel_root作为备选
        if (buttonContainer == null) {
            buttonContainer = rootView.findViewById(R.id.panel_root);
        }
        
        // 确保buttonContainer不为空
        if (buttonContainer == null) {
            return;
        }
        
        // 移除之前添加的自定义按钮
        for (int i = buttonContainer.getChildCount() - 1; i >= 0; i--) {
            View child = buttonContainer.getChildAt(i);
            if (child.getId() != R.id.panel_btn_select_all && 
                child.getId() != R.id.panel_btn_paste && 
                child.getId() != R.id.panel_btn_copy && 
                child.getId() != R.id.panel_btn_cut && 
                child.getId() != R.id.panel_btn_long_select) {
                buttonContainer.removeView(child);
            }
        }
        
        // 添加新的自定义按钮
        List<CustomButton> buttonsToAdd = editor.isEditable() ? customEditableButtons : customNonEditableButtons;
        int color = editor.getColorScheme().getColor(EditorColorScheme.TEXT_ACTION_WINDOW_ICON_COLOR);
        
        for (CustomButton customButton : buttonsToAdd) {
            ImageButton button = new ImageButton(editor.getContext());
            button.setId(customButton.id);
            button.setContentDescription(customButton.text);
            
            // 使用与自带按钮相同的布局参数
            LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                (int) (editor.getDpUnit() * 45), 
                (int) (editor.getDpUnit() * 45)
            );
            button.setLayoutParams(params);
            
            button.setScaleType(ImageView.ScaleType.CENTER);
            button.setOnClickListener(this);
            
            // 使用与自带按钮相同的背景
            TypedValue outValue = new TypedValue();
            editor.getContext().getTheme().resolveAttribute(android.R.attr.selectableItemBackground, outValue, true);
            button.setBackgroundResource(outValue.resourceId);
            
            if (customButton.drawable != null) {
                button.setImageDrawable(customButton.drawable);
            } else {
                button.setImageResource(customButton.icon);
            }
            
            applyColorFilter(button, color);
            buttonContainer.addView(button);
        }
        
        rootView.measure(View.MeasureSpec.makeMeasureSpec(1000000, View.MeasureSpec.AT_MOST), View.MeasureSpec.makeMeasureSpec(100000, View.MeasureSpec.AT_MOST));
        setSize(Math.min(rootView.getMeasuredWidth(), (int) (editor.getDpUnit() * 230)), getHeight());
    }

    @Override
    public void show() {
        if (!enabled || editor.getSnippetController().isInSnippet() || !editor.hasFocus() || editor.isInMouseMode()) {
            return;
        }
        super.show();
    }

    @Override
    public void onClick(View view) {
        int id = view.getId();
        try {
            if (id == R.id.panel_btn_select_all) {
                editor.selectAll();
                return;
            } else if (id == R.id.panel_btn_cut) {
                if (editor.getCursor().isSelected()) {
                    editor.cutText();
                }
            } else if (id == R.id.panel_btn_paste) {
                editor.pasteText();
                editor.setSelection(editor.getCursor().getRightLine(), editor.getCursor().getRightColumn());
            } else if (id == R.id.panel_btn_copy) {
                editor.copyText();
                editor.setSelection(editor.getCursor().getRightLine(), editor.getCursor().getRightColumn());
            } else if (id == R.id.panel_btn_long_select) {
                editor.beginLongSelect();
            } else {
                // 处理自定义按钮点击
                for (CustomButton customButton : customEditableButtons) {
                    if (customButton.id == id) {
                        customButton.function.run();
                        break;
                    }
                }
                for (CustomButton customButton : customNonEditableButtons) {
                    if (customButton.id == id) {
                        customButton.function.run();
                        break;
                    }
                }
            }
        } catch (Exception e) {
            // 可选：记录日志或提示用户
            e.printStackTrace();
        } finally {
            dismiss();
            editor.onTextActionWindowClick(id);
        }
    }
    
    /** 自定义按钮数据类 */
    private static class CustomButton {
        final int id;
        final String text;
        final int icon;
        final Drawable drawable;
        final Runnable function;

        // 使用资源ID的构造方法
        CustomButton(int id, String text, int icon, Runnable function) {
            this.id = id;
            this.text = text;
            this.icon = icon;
            this.drawable = null;
            this.function = function;
        }

        // 使用Drawable的构造方法
        CustomButton(int id, String text, Drawable drawable, Runnable function) {
            this.id = id;
            this.text = text;
            this.icon = 0;
            this.drawable = drawable;
            this.function = function;
        }
    }

}

