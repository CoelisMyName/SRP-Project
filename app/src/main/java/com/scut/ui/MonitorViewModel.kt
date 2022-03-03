package com.scut.ui

import android.app.Application
import android.view.View
import androidx.lifecycle.viewModelScope
import com.scut.BaseViewModel
import com.scut.SnoreRepository
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch

class MonitorViewModel(application: Application) : BaseViewModel(application) {
    private var mDurationJob: Job? = null

    private val mDetailVisibilityFlow = MutableStateFlow(View.GONE)

    private var mStartTimestamp = 0L

    init {
        viewModelScope.launch {
            var state: SnoreRepository.Message<Long> = SnoreRepository.Message.Void()
            getAudioStateFlow().collectLatest {
                if (state is SnoreRepository.Message.Start && it is SnoreRepository.Message.Stop) {
                    mDetailVisibilityFlow.emit(View.VISIBLE)
                }
                if (state is SnoreRepository.Message.Stop && it is SnoreRepository.Message.Start) {
                    mDetailVisibilityFlow.emit(View.GONE)
                }
                if (it is SnoreRepository.Message.Start) {
                    startDurationTask()
                }
                if (it is SnoreRepository.Message.Stop) {
                    stopDurationTask()
                }
                state = it
            }
        }
    }

    private val mDurationFlow = MutableStateFlow<Long>(0)

    fun getAudioStateFlow() = SnoreRepository.getAudioStateFlow()

    fun newRender(type: String) = SnoreRepository.newRender(type)

    fun getDurationFlow(): StateFlow<Long> = mDurationFlow

    fun startSnoreModule() = SnoreRepository.startSnoreModule()

    fun stopSnoreModule() = SnoreRepository.stopSnoreModule()

    fun getDetailVisibilityFlow(): StateFlow<Int> = mDetailVisibilityFlow

    private fun startDurationTask() {
        mDurationJob?.cancel()
        mDurationJob = viewModelScope.launch {
            while (isActive) {
                val duration = System.currentTimeMillis() - mStartTimestamp
                mDurationFlow.emit(duration)
                delay(200)
            }
        }
    }

    private fun stopDurationTask() {
        mDurationJob?.cancel()
        mDurationJob = null
    }
}