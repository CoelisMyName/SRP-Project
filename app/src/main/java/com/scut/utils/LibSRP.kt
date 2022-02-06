package com.scut.utils

import android.content.res.AssetManager

object LibSRP {
    init {
        System.loadLibrary("srp")
    }

    external fun setAssetManager(ast: AssetManager)
    external fun create(controller: ModuleController): Boolean
    external fun destroy(controller: ModuleController): Boolean
    external fun start(controller: ModuleController): Boolean
    external fun stop(controller: ModuleController): Boolean
    external fun getSampleRate(controller: ModuleController): Long
    external fun getStartTime(controller: ModuleController): Long
    external fun isRunning(controller: ModuleController): Boolean
    external fun registerCallback(controller: ModuleController, pointer: Long): Boolean
    external fun unregisterCallback(controller: ModuleController, pointer: Long): Boolean
}