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
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

object SnoreRepository {
    enum class AudioRecordState {
        AudioRecordIDLE, AudioRecordSTART, AudioRecordSTOP
    }

    private val mModuleController = ModuleController

    private val mRepositoryScope = CoroutineScope(SupervisorJob() + Dispatchers.Default)

    private lateinit var mDatabase: SnoreDatabase

    private lateinit var mDao: SnoreDaoWrapper

    private var mSleepRecord = SleepRecord()

    private const val TAG = "SnoreRepository"

    private var mInitFlag = false

    private class NativeRenderWrapper(render: NativeRender) : NativeRender {
        private val mRender = render
        override fun getNativePointer(): Long {
            return mRender.getNativePointer()
        }

        override fun recycle() {
            mModuleController.unregisterNativeCallback(mRender.getNativePointer())
            mRender.recycle()
        }
    }

    sealed class Message<T> {
        data class Start<T>(val timestamp: Long) : Message<T>()
        data class Package<T>(val data: T) : Message<T>()
        data class Stop<T>(val timestamp: Long) : Message<T>()
        class Void<T> : Message<T>()
    }

    private val mSnoreFlow = MutableSharedFlow<Message<Snore>>()

    private val mSPLFlow = MutableSharedFlow<Message<SPL>>()

    private val mAudioRecordStateFlow = MutableStateFlow(AudioRecordState.AudioRecordIDLE)

    fun resetAudioRecordStateFlow() {
        mRepositoryScope.launch {
            mAudioRecordStateFlow.emit(AudioRecordState.AudioRecordIDLE)
        }
    }

    fun getSnoreFlow(): SharedFlow<Message<Snore>> = mSnoreFlow

    fun getSPLFlow(): SharedFlow<Message<SPL>> = mSPLFlow

    fun getAudioRecordStateFlow(): StateFlow<AudioRecordState> = mAudioRecordStateFlow

    fun insertSleepRecord(vararg sl: SleepRecord) {
        mRepositoryScope.launch {
            mDao.insertSleepRecord(*sl)
        }
    }

    fun deleteSleepRecord(vararg sl: SleepRecord) {
        mRepositoryScope.launch {
            mDao.deleteSleepRecord(*sl)
        }
    }

    suspend fun insertSnoreRecord(vararg sr: SnoreRecord) {
        mRepositoryScope.launch {
            mDao.insertSnoreRecord(*sr)
        }
    }

    fun queryAllSleepRecord() = mDao.queryAllSleepRecord()

    fun querySleepRecordByTimestamp(timestamp: Long) = mDao.querySleepRecordByTimestamp(timestamp)

    fun querySnoreRecordByStartTime(startTime: Long) = mDao.querySnoreRecordByStartTime(startTime)

    fun updateSleepRecordDuration(timestamp: Long, duration: Long) {
        mRepositoryScope.launch {
            mDao.updateSleepRecordDuration(timestamp, duration)
        }
    }

    fun updateSleepRecordLabel(timestamp: Long, label: Double) {
        mRepositoryScope.launch {
            mDao.updateSleepRecordLabel(timestamp, label)
        }
    }

    /**
     * 用其他方法前调用
     */
    fun init(application: Application) {
        if (mInitFlag) return
        mDatabase = SnoreDatabase.getInstance(application)
        mDao = SnoreDaoWrapper(mDatabase.getSnoreDao())
        mModuleController.create(application)
        mModuleController.mSnoreCallback = object : ModuleController.SnoreCallback {
            override fun onStart(timestamp: Long) {
                mRepositoryScope.launch {
                    mSnoreFlow.emit(Message.Start(timestamp))
                    mSleepRecord = SleepRecord(timestamp, "", 0)
                    insertSleepRecord(mSleepRecord)
                }
            }

            override fun onRecognize(snore: Snore) {
                mRepositoryScope.launch {
                    mSnoreFlow.emit(Message.Package(snore))
                    val snoreRecord = SnoreRecord(
                        0,
                        snore.timestamp,
                        snore.length,
                        snore.confirm,
                        snore.startTime
                    )
                    insertSnoreRecord(snoreRecord)
                    mSleepRecord.duration = System.currentTimeMillis() - mSleepRecord.timestamp
                    updateSleepRecordDuration(mSleepRecord.timestamp, mSleepRecord.duration)
                }
            }

            override fun onStop(timestamp: Long) {
                mRepositoryScope.launch {
                    mSnoreFlow.emit(Message.Stop(timestamp))
                    mSleepRecord.duration = System.currentTimeMillis() - mSleepRecord.timestamp
                    updateSleepRecordDuration(mSleepRecord.timestamp, mSleepRecord.duration)
                }
            }
        }
        mModuleController.mSPLCallback = object : ModuleController.SPLCallback {
            override fun onStart(timestamp: Long) {
                mRepositoryScope.launch {
                    mSPLFlow.emit(Message.Start(timestamp))
                }
            }

            override fun onDetect(spl: SPL) {
                mRepositoryScope.launch {
                    mSPLFlow.emit(Message.Package(spl))
                }
            }

            override fun onStop(timestamp: Long) {
                mRepositoryScope.launch {
                    mSPLFlow.emit(Message.Stop(timestamp))
                }
            }
        }
        mModuleController.mPatientCallback = object : ModuleController.PatientCallback {
            override fun onPatientResult(timestamp: Long, label: Double) {
                updateSleepRecordLabel(timestamp, label)
            }
        }
        mInitFlag = true
    }

    fun newRender(type: String): NativeRender {
        val render = NativeRenderWrapper(RenderFactory.createRender(type))
        mModuleController.registerNativeCallback(render.getNativePointer())
        return render
    }

    fun startSnoreModule(): Boolean {
        val ret = mModuleController.start()
        mRepositoryScope.launch {
            if (ret) {
                mAudioRecordStateFlow.emit(AudioRecordState.AudioRecordSTART)
            } else {
                mAudioRecordStateFlow.emit(AudioRecordState.AudioRecordSTOP)
            }
        }
        return ret
    }

    fun stopSnoreModule(): Boolean {
        val ret = mModuleController.stop()
        mRepositoryScope.launch {
            if (ret) {
                mAudioRecordStateFlow.emit(AudioRecordState.AudioRecordSTOP)
            } else {
                mAudioRecordStateFlow.emit(AudioRecordState.AudioRecordSTART)
            }
        }
        return ret
    }
}