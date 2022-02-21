package com.scut.component

interface NativeRender {
    fun getNativePointer(): Long

    /**
     * 调用此方法，Render 将不再可用
     */
    fun recycle()
}

object RenderFactory {
    const val WAVE_RENDER = "wave"
    const val DEFAULT_RENDER = "default"

    private class NativeRenderImpl(pointer: Long) : NativeRender {
        var mPointer: Long = pointer
        override fun getNativePointer(): Long {
            return mPointer
        }

        override fun recycle() {
            deleteRender(mPointer)
            mPointer = 0
        }
    }

    fun createRender(type: String): NativeRender {
        val pointer = newRender(type)
        if (pointer == 0L) {
            throw NullPointerException("unknown render type")
        }
        return NativeRenderImpl(pointer)
    }

    private external fun newRender(type: String): Long

    private external fun deleteRender(pointer: Long)
}