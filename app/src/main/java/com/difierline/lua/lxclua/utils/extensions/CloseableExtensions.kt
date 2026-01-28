package com.difierline.lua.lxclua.utils.extensions

import java.io.Closeable

internal inline fun <T : Closeable?, R> T.useQuietly(block: (T) -> R): R? =
    try {
        this.use(block)
    } catch (t: Throwable) {
        null
    }
