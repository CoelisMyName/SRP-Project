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
    private val mModuleController = ModuleController

    private val mRepositoryScope = CoroutineScope(SupervisorJob() + Dispatchers.Default)

    private lateinit var mDatabase: SnoreDatabase

    private lateinit var mDao: SnoreDaoWrapper

    private var mSleepRecord = SleepRecord()

    private const val TAG = "SnoreRepository"

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

    fun getSnoreFlow(): SharedFlow<Message<Snore>> = mSnoreFlow

    fun getSPLFlow(): SharedFlow<Message<SPL>> = mSPLFlow

    private suspend fun insert(vararg sl: SleepRecord) = mDao.insert(*sl)

    private suspend fun update(vararg sl: SleepRecord) = mDao.update(*sl)

    private suspend fun delete(vararg sl: SleepRecord) = mDao.delete(*sl)

    private suspend fun insert(vararg sr: SnoreRecord) = mDao.insert(*sr)

    fun query() = mDao.query()

    fun query(timestamp: Long) = mDao.query(timestamp)

    /**
     * 用其他方法前调用
     */
    fun init(application: Application) {
        mDatabase = SnoreDatabase.getInstance(application)
        mDao = SnoreDaoWrapper(mDatabase.getSnoreDao())
        mModuleController.create(application)
        mModuleController.mSnoreCallback = object : ModuleController.SnoreCallback {
            override fun onStart(timestamp: Long) {
                mRepositoryScope.launch {
                    mSnoreFlow.emit(Message.Start(timestamp))
                    mSleepRecord = SleepRecord(timestamp, "", 0)
                    insert(mSleepRecord)
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
                    insert(snoreRecord)
                    mSleepRecord.duration = System.currentTimeMillis() - mSleepRecord.timestamp
                    update(mSleepRecord)
                }
            }

            override fun onStop(timestamp: Long) {
                mRepositoryScope.launch {
                    mSnoreFlow.emit(Message.Stop(timestamp))
                    mSleepRecord.duration = System.currentTimeMillis() - mSleepRecord.timestamp
                    update(mSleepRecord)
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
    }

    fun deleteSleepRecord(sl: SleepRecord) {
        mRepositoryScope.launch {
            delete(sl)
        }
    }

    fun newRender(type: String): NativeRender {
        val render = NativeRenderWrapper(RenderFactory.createRender(type))
        mModuleController.registerNativeCallback(render.getNativePointer())
        return render
    }

    fun startSnoreModule(): Boolean = mModuleController.start()

    fun stopSnoreModule(): Boolean = mModuleController.stop()
}