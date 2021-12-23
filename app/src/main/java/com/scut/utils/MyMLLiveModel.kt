package com.scut.utils

import android.content.Context
import android.os.Parcel
import android.os.Parcelable
import android.util.Log

const val TAG = "MyMLLiveModel"

/**
 * 需要读写文件权限，录音权限
 */
object MyMLLiveModel {

    init {
        System.loadLibrary("srp")
    }

//    private val m_spl: SPL = SPL()

    private var m_folder: String = "null"

    private var m_initial: Boolean = false

    var m_splAdjust: Float = 0F

    var m_start: Long = 0

    var m_sampleRate: Int = 44100

    var m_config: ModelConfig = ModelConfig()

    var m_splCallback: (SPL) -> Unit = { /*Log.d(TAG, "m_splCallback(): get called")*/ }

    /**
     * numMinute start end
     */
    var m_clsCallback: (Long, Long, Long, Boolean) -> Unit =
        { _, _, _, _ -> Log.d(TAG, "m_clsCallback(): get called") }

    private fun calculateSPL(spl: SPL): Unit {
        //Log.d(TAG, "calculateSPL(): $spl")
        m_splCallback(spl)
    }

    private fun calculateCLS(min: Long, start: Long, end: Long, pos: Boolean): Unit {
        Log.d(TAG, "calculateCLS: " + min + "min " + start + " " + end + " " + pos)
        m_clsCallback(min, start, end, pos)
    }

    fun isInitialed(): Boolean {
        return m_initial
    }

    fun initial(context: Context): Boolean {
        val file = context.externalCacheDir
        if (file == null) {
            Log.d(TAG, "initial: get external cache dir null")
            return false
        }
        m_folder = file.absolutePath;
        if (!init()) return false
        m_initial = true
        return true
    }

    external fun start(): Boolean

    external fun stop(): Boolean

    private external fun init(): Boolean
}

data class ModelConfig(
    val savePerMinute: Boolean = false,
    val savePositive: Boolean = false,
    val saveFolder: String = ""
) : Parcelable {

    constructor(parcel: Parcel) : this(
        parcel.readByte() != 0.toByte(),
        parcel.readByte() != 0.toByte(),
        parcel.readString() ?: ""
    ) {
    }

    override fun writeToParcel(parcel: Parcel, flags: Int) {
        parcel.writeByte(if (savePerMinute) 1 else 0)
        parcel.writeByte(if (savePositive) 1 else 0)
        parcel.writeString(saveFolder)
    }

    override fun describeContents(): Int {
        return 0
    }

    companion object CREATOR : Parcelable.Creator<ModelConfig> {
        override fun createFromParcel(parcel: Parcel): ModelConfig {
            return ModelConfig(parcel)
        }

        override fun newArray(size: Int): Array<ModelConfig?> {
            return arrayOfNulls(size)
        }
    }
}