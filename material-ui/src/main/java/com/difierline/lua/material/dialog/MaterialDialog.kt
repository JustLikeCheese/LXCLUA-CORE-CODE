package com.difierline.lua.material.dialog

import android.content.Context
import android.content.DialogInterface
import android.graphics.drawable.Drawable
import android.view.View
import androidx.annotation.ArrayRes
import androidx.annotation.DrawableRes
import androidx.annotation.LayoutRes
import androidx.annotation.StringRes
import androidx.appcompat.app.AlertDialog
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import com.difierline.lua.material.R
import com.difierline.lua.material.Material

class MaterialDialog private constructor(context: Context) {

    private val builder: MaterialAlertDialogBuilder
    private var dialog: AlertDialog? = null
    private var onDismissListener: (() -> Unit)? = null
    private var onCancelListener: (() -> Unit)? = null
    private var onShowListener: (() -> Unit)? = null

    init {
        builder = MaterialAlertDialogBuilder(context)
    }

    companion object {
        fun create(context: Context): MaterialDialog {
            return MaterialDialog(context)
        }
    }

    fun setTitle(title: String): MaterialDialog {
        builder.setTitle(title)
        return this
    }

    fun setTitle(@StringRes titleResId: Int): MaterialDialog {
        builder.setTitle(titleResId)
        return this
    }

    fun setMessage(message: String): MaterialDialog {
        builder.setMessage(message)
        return this
    }

    fun setMessage(@StringRes messageResId: Int): MaterialDialog {
        builder.setMessage(messageResId)
        return this
    }

    fun setIcon(drawable: Drawable): MaterialDialog {
        builder.setIcon(drawable)
        return this
    }

    fun setIcon(@DrawableRes iconResId: Int): MaterialDialog {
        builder.setIcon(iconResId)
        return this
    }

    fun setPositiveButton(text: String, listener: (() -> Unit)? = null): MaterialDialog {
        builder.setPositiveButton(text) { _, _ ->
            listener?.invoke()
        }
        return this
    }

    fun setPositiveButton(@StringRes textResId: Int, listener: (() -> Unit)? = null): MaterialDialog {
        builder.setPositiveButton(textResId) { _, _ ->
            listener?.invoke()
        }
        return this
    }

    fun setNegativeButton(text: String, listener: (() -> Unit)? = null): MaterialDialog {
        builder.setNegativeButton(text) { _, _ ->
            listener?.invoke()
        }
        return this
    }

    fun setNegativeButton(@StringRes textResId: Int, listener: (() -> Unit)? = null): MaterialDialog {
        builder.setNegativeButton(textResId) { _, _ ->
            listener?.invoke()
        }
        return this
    }

    fun setNeutralButton(text: String, listener: (() -> Unit)? = null): MaterialDialog {
        builder.setNeutralButton(text) { _, _ ->
            listener?.invoke()
        }
        return this
    }

    fun setNeutralButton(@StringRes textResId: Int, listener: (() -> Unit)? = null): MaterialDialog {
        builder.setNeutralButton(textResId) { _, _ ->
            listener?.invoke()
        }
        return this
    }

    fun setItems(items: Array<String>, listener: (dialog: AlertDialog, which: Int) -> Unit): MaterialDialog {
        builder.setItems(items) { _, which ->
            listener(dialog!!, which)
        }
        return this
    }

    fun setItems(@ArrayRes itemsResId: Int, listener: (dialog: AlertDialog, which: Int) -> Unit): MaterialDialog {
        builder.setItems(itemsResId) { _, which ->
            listener(dialog!!, which)
        }
        return this
    }

    fun setSingleChoiceItems(
        items: Array<String>,
        checkedItem: Int = -1,
        listener: (dialog: AlertDialog, which: Int) -> Unit
    ): MaterialDialog {
        builder.setSingleChoiceItems(items, checkedItem) { _, which ->
            listener(dialog!!, which)
        }
        return this
    }

    fun setSingleChoiceItems(
        @ArrayRes itemsResId: Int,
        checkedItem: Int = -1,
        listener: (dialog: AlertDialog, which: Int) -> Unit
    ): MaterialDialog {
        builder.setSingleChoiceItems(itemsResId, checkedItem) { _, which ->
            listener(dialog!!, which)
        }
        return this
    }

    fun setMultiChoiceItems(
        items: Array<String>,
        checkedItems: BooleanArray,
        listener: (dialog: AlertDialog, which: Int, isChecked: Boolean) -> Unit
    ): MaterialDialog {
        builder.setMultiChoiceItems(items, checkedItems) { _, which, isChecked ->
            listener(dialog!!, which, isChecked)
        }
        return this
    }

    fun setMultiChoiceItems(
        @ArrayRes itemsResId: Int,
        checkedItems: BooleanArray,
        listener: (dialog: AlertDialog, which: Int, isChecked: Boolean) -> Unit
    ): MaterialDialog {
        builder.setMultiChoiceItems(itemsResId, checkedItems) { _, which, isChecked ->
            listener(dialog!!, which, isChecked)
        }
        return this
    }

    fun setView(view: View): MaterialDialog {
        builder.setView(view)
        return this
    }

    fun setView(@LayoutRes layoutResId: Int): MaterialDialog {
        builder.setView(layoutResId)
        return this
    }

    fun setCustomTitle(customTitle: View): MaterialDialog {
        builder.setCustomTitle(customTitle)
        return this
    }

    fun setCancelable(cancelable: Boolean): MaterialDialog {
        builder.setCancelable(cancelable)
        return this
    }

    fun setOnCancelListener(listener: () -> Unit): MaterialDialog {
        onCancelListener = listener
        builder.setOnCancelListener { listener.invoke() }
        return this
    }

    fun setOnDismissListener(listener: () -> Unit): MaterialDialog {
        onDismissListener = listener
        builder.setOnDismissListener { listener.invoke() }
        return this
    }

    fun setOnShowListener(listener: () -> Unit): MaterialDialog {
        onShowListener = listener
        return this
    }

    fun setOnKeyListener(listener: (dialog: DialogInterface, keyCode: Int, event: android.view.KeyEvent) -> Boolean): MaterialDialog {
        builder.setOnKeyListener(listener)
        return this
    }

    fun show(): AlertDialog {
        dialog = builder.create()
        dialog?.show()
        onShowListener?.invoke()
        return dialog!!
    }

    fun getDialog(): AlertDialog? = dialog

    fun dismiss() {
        dialog?.dismiss()
    }

    fun hide() {
        dialog?.hide()
    }

    fun getButton(whichButton: Int): android.widget.Button? {
        return dialog?.getButton(whichButton)
    }

    fun getCustomView(): View? {
        return dialog?.findViewById(android.R.id.custom)
    }

    fun getContentView(): View? {
        return dialog?.findViewById(android.R.id.content)
    }

    object StaticMethods {
        fun showConfirmDialog(
            context: Context,
            title: String,
            message: String,
            positiveText: String = "确定",
            negativeText: String? = "取消",
            onPositiveClick: () -> Unit,
            onNegativeClick: (() -> Unit)? = null
        ): AlertDialog {
            val dialog = create(context)
                .setTitle(title)
                .setMessage(message)
                .setPositiveButton(positiveText, onPositiveClick)
            negativeText?.let {
                dialog.setNegativeButton(it, onNegativeClick)
            }
            return dialog.show()
        }

        fun showMessageDialog(
            context: Context,
            title: String,
            message: String,
            buttonText: String = "确定",
            onButtonClick: (() -> Unit)? = null
        ): AlertDialog {
            return create(context)
                .setTitle(title)
                .setMessage(message)
                .setPositiveButton(buttonText, onButtonClick)
                .show()
        }

        fun showListDialog(
            context: Context,
            title: String,
            items: Array<String>,
            onItemClick: (which: Int) -> Unit
        ): AlertDialog {
            return create(context)
                .setTitle(title)
                .setItems(items) { _, which ->
                    onItemClick(which)
                }
                .show()
        }

        fun showSingleChoiceDialog(
            context: Context,
            title: String,
            items: Array<String>,
            checkedItem: Int = -1,
            onItemClick: (which: Int) -> Unit
        ): AlertDialog {
            return create(context)
                .setTitle(title)
                .setSingleChoiceItems(items, checkedItem) { _, which ->
                    onItemClick(which)
                }
                .setPositiveButton("确定", null)
                .setNegativeButton("取消", null)
                .show()
        }

        fun showMultiChoiceDialog(
            context: Context,
            title: String,
            items: Array<String>,
            checkedItems: BooleanArray,
            onMultiChoiceClick: (which: Int, isChecked: Boolean) -> Unit
        ): AlertDialog {
            return create(context)
                .setTitle(title)
                .setMultiChoiceItems(items, checkedItems) { _, which, isChecked ->
                    onMultiChoiceClick(which, isChecked)
                }
                .setPositiveButton("确定", null)
                .setNegativeButton("取消", null)
                .show()
        }

        fun showCustomViewDialog(
            context: Context,
            title: String,
            view: View,
            positiveText: String? = "确定",
            negativeText: String? = "取消",
            onPositiveClick: (() -> Unit)? = null,
            onNegativeClick: (() -> Unit)? = null
        ): AlertDialog {
            val dialog = create(context)
                .setTitle(title)
                .setView(view)

            positiveText?.let {
                dialog.setPositiveButton(it, onPositiveClick)
            }

            negativeText?.let {
                dialog.setNegativeButton(it, onNegativeClick)
            }

            return dialog.show()
        }
    }
}

class MaterialLoadingDialog private constructor(context: Context) {

    private val dialogContext: Context
    private val dialog: AlertDialog
    private val progressBar: com.google.android.material.progressindicator.CircularProgressIndicator
    private val messageView: android.widget.TextView
    private val builder: MaterialAlertDialogBuilder

    init {
        dialogContext = context
        val view = View.inflate(context, R.layout.dialog_progress, null)
        progressBar = view.findViewById(R.id.progress_bar)
        messageView = view.findViewById(R.id.progress_text)

        builder = MaterialAlertDialogBuilder(context)
            .setView(view)
            .setCancelable(false)

        dialog = builder.create()
    }

    companion object {
        fun create(context: Context): MaterialLoadingDialog {
            return MaterialLoadingDialog(context)
        }
    }

    fun setMessage(message: String): MaterialLoadingDialog {
        messageView.text = message
        messageView.visibility = if (message.isNotEmpty()) View.VISIBLE else View.GONE
        return this
    }

    fun setMessage(@StringRes messageResId: Int): MaterialLoadingDialog {
        return setMessage(dialogContext.getString(messageResId))
    }

    fun setProgress(progress: Int): MaterialLoadingDialog {
        progressBar.isIndeterminate = false
        progressBar.progress = progress
        return this
    }

    fun setMaxProgress(maxProgress: Int): MaterialLoadingDialog {
        progressBar.max = maxProgress
        return this
    }

    fun setIndeterminate(indeterminate: Boolean): MaterialLoadingDialog {
        progressBar.isIndeterminate = indeterminate
        return this
    }

    fun setProgressColor(color: Int): MaterialLoadingDialog {
        progressBar.setIndicatorColor(color)
        return this
    }

    fun setCancelable(cancelable: Boolean): MaterialLoadingDialog {
        dialog.setCancelable(cancelable)
        return this
    }

    fun setOnCancelListener(onCancel: () -> Unit): MaterialLoadingDialog {
        dialog.setOnCancelListener { onCancel() }
        return this
    }

    fun show() {
        dialog.show()
    }

    fun dismiss() {
        dialog.dismiss()
    }

    fun updateProgress(progress: Int) {
        progressBar.progress = progress
    }

    fun incrementProgress(increment: Int) {
        progressBar.progress = progressBar.progress + increment
    }
}
