package com.scut.utils

object ModuleController {
    interface SnoreCallback {
        fun onStart(timestamp: Long) {}
        fun onRecognize(snore: Snore) {}
        fun onStop(timestamp: Long) {}
    }

    interface SPLCallback {
        fun onStart(timestamp: Long) {}
        fun onDetect(spl: SPL) {}
        fun onStop(timestamp: Long) {}
    }

    class DefaultSnoreCallback : SnoreCallback
    class DefaultSPLCallback : SPLCallback

    init {
        System.loadLibrary("srp")
    }

    private const val TAG = "ModuleController"

    var mSnoreCallback = DefaultSnoreCallback()

    var mSPLCallback = DefaultSPLCallback()

    fun resetSnoreCallback() {
        mSnoreCallback = DefaultSnoreCallback()
    }

    fun resetSPLCallback() {
        mSPLCallback = DefaultSPLCallback()
    }

    fun registerNativeCallback(pointer: Long): Boolean {
        return LibSRP.registerCallback(this, pointer)
    }

    fun unregisterNativeCallback(pointer: Long): Boolean {
        return LibSRP.unregisterCallback(this, pointer)
    }

    private fun onSnoreStart(timestamp: Long) {
        mSnoreCallback.onStart(timestamp)
    }

    private fun onSnoreStop(timestamp: Long) {
        mSnoreCallback.onStop(timestamp)
    }

    private fun onSPLStart(timestamp: Long) {
        mSPLCallback.onStart(timestamp)
    }

    private fun onSPLStop(timestamp: Long) {
        mSPLCallback.onStop(timestamp)
    }

    private fun onSnoreRecognize(snore: Snore) {
        mSnoreCallback.onRecognize(snore)
    }

    private fun onSPLDetect(spl: SPL) {
        mSPLCallback.onDetect(spl)
    }

    fun create(): Boolean {
        return LibSRP.create(this)
    }

    fun destroy(): Boolean {
        return LibSRP.destroy(this)
    }

    fun start(): Boolean {
        return LibSRP.start(this)
    }

    fun stop(): Boolean {
        return LibSRP.stop(this)
    }

    fun getSampleRate(): Long {
        return LibSRP.getSampleRate(this)
    }

    fun getStartTime(): Long {
        return LibSRP.getStartTime(this)
    }

    fun isRunning(): Boolean {
        return LibSRP.isRunning(this)
    }
}