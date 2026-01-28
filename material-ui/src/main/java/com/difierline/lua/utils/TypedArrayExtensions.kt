package com.difierline.lua.utils

import android.content.res.TypedArray

fun TypedArray.useAndRecycle(block: (attributes: TypedArray) -> Unit) {
    block(this)
    recycle()
}