package com.scut.component

interface NativeRender {
    fun create()

    fun getNativePointer(): Long;

    fun destroy()
}