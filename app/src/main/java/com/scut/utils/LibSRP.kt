package com.scut.utils

object LibSRP {
    init {
        System.loadLibrary("srp")
    }

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