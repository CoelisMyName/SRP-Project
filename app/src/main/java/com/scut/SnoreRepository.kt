package com.scut

import android.app.Application
import com.scut.component.NativeRender
import com.scut.component.RenderFactory
import com.scut.dao.SnoreDaoWrapper
import com.scut.model.SleepRecord
import com.scut.model.SnoreRecord
import com.scut.utils.ModuleController
import com.scut.utils.SPL
import com.scut.utils.Snore
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.launch

object SnoreRepository {
    private val mSnoreCallback = object : ModuleController.SnoreCallback {
        override fun onStart(timestamp: Long) {
            mRepositoryScope.launch {
                mSnoreFlow.emit(Message.Start(timestamp))
            }
        }

        override fun onRecognize(snore: Snore) {
            mRepositoryScope.launch {
                if (!snore.confirm) return@launch
                mSnoreFlow.emit(Message.Package(snore))
                val snoreRecord = SnoreRecord(
                    0,
                    snore.timestamp,
                    snore.length,
                    snore.confirm,
                    snore.startTime
                )
                mDao.insertSnoreRecord(snoreRecord)
                mDao.updateSleepRecordSnoreTimesIncrement(snoreRecord.startTime, 1L)
                mDao.updateSleepRecordSnoreTotalDurationIncrement(
                    snoreRecord.startTime,
                    snoreRecord.length
                )
            }
        }

        override fun onStop(timestamp: Long) {
            mRepositoryScope.launch {
                mSnoreFlow.emit(Message.Stop(timestamp))
            }
        }
    }
    private val mSPLCallback = object : ModuleController.SPLCallback {
        private var mStartTimestamp: Long = 0
        private var mLastUpdate: Long = 0
        override fun onStart(timestamp: Long) {
            mStartTimestamp = timestamp
            mLastUpdate = timestamp
            mRepositoryScope.launch {
                mSPLFlow.emit(Message.Start(timestamp))
                mAudioStateFlow.emit(Message.Start(timestamp))
                mDao.insertSleepRecord(SleepRecord(timestamp, ""))
            }
        }

        override fun onDetect(spl: SPL) {
            val startTimestamp = mStartTimestamp
            val duration = spl.timestamp - mStartTimestamp
            mRepositoryScope.launch {
                mSPLFlow.emit(Message.Package(spl))
                mDao.updateSleepRecordDuration(startTimestamp, duration)
            }
        }

        override fun onStop(timestamp: Long) {
            mLastUpdate = timestamp
            val startTimestamp = mStartTimestamp
            val duration = timestamp - startTimestamp
            mRepositoryScope.launch {
                mSPLFlow.emit(Message.Stop(timestamp))
                mAudioStateFlow.emit(Message.Stop(timestamp))
                mDao.updateSleepRecordDuration(startTimestamp, duration)
            }
        }
    }
    private val mPatientCallback = object : ModuleController.PatientCallback {
        override fun onPatientResult(timestamp: Long, label: Double) {
            mRepositoryScope.launch {
                mDao.updateSleepRecordLabel(timestamp, label)
            }
        }
    }
    private val mRepositoryScope = CoroutineScope(SupervisorJob() + Dispatchers.Default)
    private val mSnoreFlow = MutableSharedFlow<Message<Snore>>()
    private val mSPLFlow = MutableSharedFlow<Message<SPL>>()
    private val mAudioStateFlow = MutableSharedFlow<Message<Long>>(2)
    private lateinit var mDatabase: SnoreDatabase
    private lateinit var mDao: SnoreDaoWrapper
    private var mInitFlag = false

    private class NativeRenderWrapper(render: NativeRender) : NativeRender {
        private val mRender = render
        override fun getNativePointer(): Long {
            return mRender.getNativePointer()
        }

        override fun recycle() {
            ModuleController.unregisterNativeCallback(mRender.getNativePointer())
            mRender.recycle()
        }
    }

    sealed class Message<T> {
        data class Start<T>(val timestamp: Long) : Message<T>()
        data class Package<T>(val data: T) : Message<T>()
        data class Stop<T>(val timestamp: Long) : Message<T>()
        class Void<T> : Message<T>()
    }

    fun getSnoreFlow(): SharedFlow<Message<Snore>> = mSnoreFlow

    fun getSPLFlow(): SharedFlow<Message<SPL>> = mSPLFlow

    fun getAudioStateFlow(): SharedFlow<Message<Long>> = mAudioStateFlow

    fun deleteSleepRecord(vararg sl: SleepRecord) {
        mRepositoryScope.launch {
            mDao.deleteSleepRecord(*sl)
        }
    }

    fun queryAllSleepRecord() = mDao.queryAllSleepRecord()

    fun querySleepRecordByTimestamp(timestamp: Long) = mDao.querySleepRecordByTimestamp(timestamp)

    fun querySnoreRecordByStartTime(startTime: Long) = mDao.querySnoreRecordByStartTime(startTime)

    /**
     * 用其他方法前调用
     */
    fun init(application: Application) {
        if (mInitFlag) return
        mDatabase = SnoreDatabase.getInstance(application)
        mDao = SnoreDaoWrapper(mDatabase.getSnoreDao())
        ModuleController.create(application)
        //注册监听器
        ModuleController.mSnoreCallback = mSnoreCallback
        ModuleController.mSPLCallback = mSPLCallback
        ModuleController.mPatientCallback = mPatientCallback
        mInitFlag = true
    }

    fun newRender(type: String): NativeRender {
        val render = NativeRenderWrapper(RenderFactory.createRender(type))
        ModuleController.registerNativeCallback(render.getNativePointer())
        return render
    }

    fun startSnoreModule() = ModuleController.start()

    fun stopSnoreModule() = ModuleController.stop()
}