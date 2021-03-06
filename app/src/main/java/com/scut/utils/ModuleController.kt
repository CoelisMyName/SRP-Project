package com.scut.utils

import android.Manifest
import android.app.Application
import android.util.Log

object ModuleController {
    interface SnoreCallback {
        //启动时间戳
        fun onStart(timestamp: Long) {}

        //鼾声数据
        fun onRecognize(snore: Snore) {}

        //停止时间戳
        fun onStop(timestamp: Long) {}
    }

    interface SPLCallback {
        //启动时间戳
        fun onStart(timestamp: Long) {}

        //SPL 声压级数据
        fun onDetect(spl: SPL) {}

        //停止时间戳
        fun onStop(timestamp: Long) {}
    }

    interface PatientCallback {
        //timestamp 音频开始时间戳，label 识别
        fun onPatientResult(timestamp: Long, label: Double) {}
    }

    data class SnoreConfig(
        var saveWhole: Boolean = false,
        var savePerMinute: Boolean = false,
        var saveSegment: Boolean = false,
        var savePositive: Boolean = false
    )

    class DefaultSnoreCallback : SnoreCallback
    class DefaultSPLCallback : SPLCallback
    class DefaultPatientCallback : PatientCallback

    init {
        System.loadLibrary("srp")
    }

    private const val TAG = "ModuleController"

    private lateinit var mApplication: Application

    private var mInit = false

    private var mGranted = true

    private val mPermissions = arrayOf(
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.RECORD_AUDIO,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    )

    var mSnoreCallback: SnoreCallback = DefaultSnoreCallback()

    var mSPLCallback: SPLCallback = DefaultSPLCallback()

    var mPatientCallback: PatientCallback = DefaultPatientCallback()

    fun resetSnoreCallback() {
        mSnoreCallback = DefaultSnoreCallback()
    }

    fun resetSPLCallback() {
        mSPLCallback = DefaultSPLCallback()
    }

    fun resetPatientCallback() {
        mPatientCallback = DefaultPatientCallback()
    }

    fun registerNativeCallback(pointer: Long): Boolean = LibSRP.registerAudioGLRender(this, pointer)

    fun unregisterNativeCallback(pointer: Long): Boolean =
        LibSRP.unregisterAudioGLRender(this, pointer)

    private fun onSnoreStart(timestamp: Long) = mSnoreCallback.onStart(timestamp)

    private fun onSnoreStop(timestamp: Long) = mSnoreCallback.onStop(timestamp)

    private fun onSPLStart(timestamp: Long) = mSPLCallback.onStart(timestamp)

    private fun onSPLStop(timestamp: Long) = mSPLCallback.onStop(timestamp)

    private fun onSnoreRecognize(snore: Snore) = mSnoreCallback.onRecognize(snore)

    private fun onSPLDetect(spl: SPL) = mSPLCallback.onDetect(spl)

    private fun onPatientResult(timestamp: Long, label: Double) =
        mPatientCallback.onPatientResult(timestamp, label)

    /**
     * 调用会检查权限，如果没有权限则返回 false，如果本地初始化没有成功也会返回 false
     */
    fun create(application: Application): Boolean {
        if (mInit) return true
        mApplication = application
        Log.d(TAG, "create: application $application")
        mInit = LibSRP.create(this)
        return mInit
    }

    fun destroy(): Boolean {
        if (!mInit) return false
        mInit = false
        return LibSRP.destroy(this)
    }

    fun start(): Boolean {
        if (!mInit) return false
        mGranted = Utils.permissionGranted(mApplication, mPermissions)
        if (!mGranted) return false
        mApplication.externalCacheDir ?: return false
        return LibSRP.start(this)
    }

    fun stop(): Boolean {
        if (!mInit) return false
        return LibSRP.stop(this)
    }

    fun getSampleRate(): Long = LibSRP.getSampleRate(this)

    fun getStartTime(): Long = LibSRP.getStartTime(this)

    fun isRunning(): Boolean = LibSRP.isRunning(this)
}