package com.scut.component

import android.content.Context
import android.graphics.SurfaceTexture
import android.util.AttributeSet
import android.util.Log
import android.view.TextureView

open class NativeTextureView : TextureView, TextureView.SurfaceTextureListener {
    companion object {
        const val TAG = "NativeTextureView"
    }
    constructor(context: Context) : super(context) {
    }
    constructor(context: Context, attrs: AttributeSet) : super(context, attrs) {
    }
    private class NativeGLThread(view: NativeTextureView, render: NativeRender) {
        companion object {
            const val TAG = "NativeGLThread"
        }

        private val mRender: NativeRender
        private val mThread: Long

        init {
            if (render.getNativePointer() == 0L) {
                throw NullPointerException("native render pointer is nullptr")
            }
            mRender = render
            mThread = LibGLThread.create(view, mRender.getNativePointer())
        }

        fun surfaceCreate(surface: SurfaceTexture, width: Int, height: Int) {
            LibGLThread.surfaceCreate(mThread, surface, width, height)
        }

        fun surfaceSizeChanged(surface: SurfaceTexture, width: Int, height: Int) {
            LibGLThread.surfaceSizeChanged(mThread, surface, width, height)
        }

        fun surfaceDestroyed(): Boolean {
            return LibGLThread.surfaceDestroyed(mThread)
        }

        fun surfaceUpdated(surface: SurfaceTexture) {
            LibGLThread.surfaceUpdated(mThread, surface)
        }

        fun onStart() {
            LibGLThread.onStart(mThread)
        }

        fun onStop() {
            LibGLThread.onStop(mThread)
        }

        fun onResume() {
            LibGLThread.onResume(mThread)
        }

        fun onPause() {
            LibGLThread.onPause(mThread)
        }

        protected fun finalize() {
            LibGLThread.destroy(mThread)
            RenderFactory.destroyRender(mRender)
            Log.d(TAG, "finalize: ")
        }
    }
    private var mFlagInit: Boolean = false
    private lateinit var mThread: NativeGLThread
    private lateinit var mRender: NativeRender

    init {
        surfaceTextureListener = this
    }

    /**
     * 必须在 onCreate 时候调用，否则不显示
     */
    fun setRender(type: String) {
        if (mFlagInit) return
        mRender = RenderFactory.createRender(type)
        mThread = NativeGLThread(this, mRender)
        mFlagInit = true
    }

    fun getRender(): NativeRender {
        return mRender
    }

    override fun onSurfaceTextureAvailable(st: SurfaceTexture, width: Int, height: Int) {
        if (!mFlagInit) return
        Log.d(TAG, "onSurfaceTextureAvailable: width = $width, height = $height")
        mThread.surfaceCreate(st, width, height)
    }

    override fun onSurfaceTextureSizeChanged(st: SurfaceTexture, width: Int, height: Int) {
        if (!mFlagInit) return
        Log.d(TAG, "onSurfaceTextureSizeChanged: width = $width, height = $height")
        mThread.surfaceSizeChanged(st, width, height)
    }

    override fun onSurfaceTextureDestroyed(st: SurfaceTexture): Boolean {
        if (!mFlagInit) return true
        Log.d(TAG, "onSurfaceTextureDestroyed: ")
        return mThread.surfaceDestroyed()
    }

    override fun onSurfaceTextureUpdated(st: SurfaceTexture) {
        if (!mFlagInit) return
//        Log.d(TAG, "onSurfaceTextureUpdated: ")
        mThread.surfaceUpdated(st)
    }

    override fun onAttachedToWindow() {
        Log.d(TAG, "onAttachedToWindow: ")
        super.onAttachedToWindow()
    }

    override fun onDetachedFromWindow() {
        Log.d(TAG, "onDetachedFromWindow: ")
        super.onDetachedFromWindow()
    }

    fun onStart() {
        mThread.onStart()
    }

    fun onStop() {
        mThread.onStop()
    }

    fun onResume() {
        mThread.onResume()
    }

    fun onPause() {
        mThread.onPause()
    }
}