package com.scut.ui

import android.app.Application
import androidx.lifecycle.viewModelScope
import com.scut.BaseViewModel
import com.scut.SnoreRepository
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.onEach
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch

class MonitorViewModel(application: Application) : BaseViewModel(application) {
    private var mDurationJob: Job? = null
    private var mAudioRecordState = SnoreRepository.AudioRecordState.AudioRecordIDLE

    init {
        viewModelScope.launch {
            getAudioRecordStateFlow().onEach {
                if (it == SnoreRepository.AudioRecordState.AudioRecordSTART) {
                    startDurationTask()
                }
                if (it == SnoreRepository.AudioRecordState.AudioRecordSTART) {
                    stopDurationTask()
                }
            }.collect()
        }
    }

    override fun onCleared() {
        super.onCleared()
        if (mAudioRecordState == SnoreRepository.AudioRecordState.AudioRecordIDLE) {
            SnoreRepository.resetAudioRecordStateFlow()
        }
    }

    private val mDurationFlow = MutableStateFlow<Long>(0)

    fun getAudioRecordStateFlow() = SnoreRepository.getAudioRecordStateFlow()

    fun newRender(type: String) = SnoreRepository.newRender(type)

    fun getDurationFlow(): StateFlow<Long> = mDurationFlow

    fun startSnoreModule() = SnoreRepository.startSnoreModule()

    fun stopSnoreModule() = SnoreRepository.stopSnoreModule()

    private fun startDurationTask() {
        mDurationJob?.cancel()
        mDurationJob = viewModelScope.launch {
            val startTime = System.currentTimeMillis()
            while (isActive) {
                val duration = System.currentTimeMillis() - startTime
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