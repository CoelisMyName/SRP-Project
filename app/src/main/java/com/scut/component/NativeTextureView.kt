package com.scut.component

import android.content.Context
import android.graphics.SurfaceTexture
import android.util.AttributeSet
import android.util.Log
import android.view.TextureView

open class NativeTextureView : TextureView, TextureView.SurfaceTextureListener {
    companion object {
        const val TAG = "NativeTextureView"

        class NativeGLThread(view: NativeTextureView, render: NativeRender) {
            companion object {
                const val TAG = "NativeGLThread"
            }

            private val mRender: Long
            private val mThread: Long

            init {
                if (render.getNativePointer() == 0L) {
                    throw NullPointerException("native render pointer is nullptr")
                }
                mRender = render.getNativePointer()
                mThread = LibGLThread.create(view, mRender)
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

            protected fun finalize() {
                LibGLThread.destroy(mThread)
            }
        }
    }

    constructor(context: Context, attrs: AttributeSet) : super(context, attrs) {
    }

    constructor(context: Context) : super(context) {
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
    fun setRender(render: NativeRender) {
        mRender = render
        mThread = NativeGLThread(this, mRender)
        mFlagInit = true
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
        Log.d(TAG, "onSurfaceTextureUpdated: ")
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

    fun onResume() {
        Log.d(TAG, "onResume: ")
    }

    fun onPause() {
        Log.d(TAG, "onPause: ")
    }
}