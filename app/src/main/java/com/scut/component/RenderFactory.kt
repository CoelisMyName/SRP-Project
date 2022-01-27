package com.scut.component

object RenderFactory {
    const val WAVE_RENDER = "wave"
    const val DEFAULT_RENDER = "default"

    private class RenderImpl(pointer: Long) : NativeRender {
        var mPointer: Long = pointer
        override fun getNativePointer(): Long {
            return mPointer
        }
    }

    fun createRender(type: String): NativeRender {
        val pointer = newRender(type)
        if (pointer == 0L) {
            throw NullPointerException("unknown render type")
        }
        return RenderImpl(pointer)
    }

    fun destroyRender(render: NativeRender) {
        val r = render as RenderImpl
        deleteRender(r.mPointer)
        r.mPointer = 0
    }

    private external fun newRender(type: String): Long

    private external fun deleteRender(pointer: Long)
}