package com.scut.utils

object SnoringRecognition {
    init {
        System.loadLibrary("srp")
    }

    private const val TAG = "SnoringRecognition";

    var m_splCallback: (SPL) -> Unit = { /*Log.d(TAG, "m_splCallback(): get called")*/ }

    private fun onRecognize(minute: Long, start_ms: Long, end_ms: Long, positive: Boolean): Unit {

    }

    private fun onSPLDetect(spl: SPL): Unit {
        //Log.d(TAG, "onSPLDetect: $spl")
    }

    fun start(): Boolean {
        return nativeStart();
    }

    fun stop(): Boolean {
        return nativeStop();
    }

    fun getSampleRate(): Double {
        return nativeGetSampleRate()
    }

    fun getStartTime(): Long {
        return nativeGetStartTime()
    }

    fun isRunning(): Boolean {
        return nativeIsRunning()
    }

    private external fun nativeStart(): Boolean

    private external fun nativeStop(): Boolean

    private external fun nativeGetSampleRate(): Double

    private external fun nativeGetStartTime(): Long

    private external fun nativeIsRunning(): Boolean
}