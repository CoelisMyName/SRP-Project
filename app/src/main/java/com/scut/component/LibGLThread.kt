package com.scut.component

import android.graphics.SurfaceTexture

object LibGLThread {
    init {
        System.loadLibrary("srp")
    }

    external fun create(view: NativeTextureView, render: Long): Long
    external fun surfaceCreate(thread: Long, surface: SurfaceTexture, width: Int, height: Int)
    external fun surfaceSizeChanged(thread: Long, surface: SurfaceTexture, width: Int, height: Int)
    external fun surfaceDestroyed(thread: Long): Boolean
    external fun surfaceUpdated(thread: Long, surface: SurfaceTexture)
    external fun destroy(thread: Long)
    external fun onStart(thread: Long)
    external fun onStop(thread: Long)
    external fun onResume(thread: Long)
    external fun onPause(thread: Long)
}