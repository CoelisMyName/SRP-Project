package com.scut

import android.app.Application
import com.scut.component.NativeRender
import com.scut.component.RenderFactory
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

    fun init(application: Application) {
        mModuleController.create(application)
        mModuleController.mSnoreCallback = object : ModuleController.SnoreCallback {
            override fun onStart(timestamp: Long) {
                mRepositoryScope.launch {
                    mSnoreFlow.emit(Message.Start(timestamp))
                }
            }

            override fun onRecognize(snore: Snore) {
                mRepositoryScope.launch {
                    mSnoreFlow.emit(Message.Package(snore))
                }
            }

            override fun onStop(timestamp: Long) {
                mRepositoryScope.launch {
                    mSnoreFlow.emit(Message.Stop(timestamp))
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

    fun newRender(type: String): NativeRender {
        val render = NativeRenderWrapper(RenderFactory.createRender(type))
        mModuleController.registerNativeCallback(render.getNativePointer())
        return render
    }

    fun startSnoreModule(): Boolean = mModuleController.start()

    fun stopSnoreModule(): Boolean = mModuleController.stop()
}