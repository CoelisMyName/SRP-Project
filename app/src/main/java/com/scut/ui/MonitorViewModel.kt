package com.scut.ui

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.scut.SnoreRepository
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.onEach
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch

class MonitorViewModel(application: Application) : AndroidViewModel(application) {
    private val mSnoreRepository = SnoreRepository

    private var mDurationJob: Job? = null

    init {
        mSnoreRepository.init(application)
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

    private val mDurationFlow = MutableStateFlow<Long>(0)

    fun getAudioRecordStateFlow() = mSnoreRepository.getAudioRecordStateFlow()

    fun newRender(type: String) = mSnoreRepository.newRender(type)

    fun getDurationFlow(): StateFlow<Long> = mDurationFlow

    fun resetAudioRecordStateFlow() = mSnoreRepository.resetAudioRecordStateFlow()

    fun startSnoreModule() = mSnoreRepository.startSnoreModule()

    fun stopSnoreModule() = mSnoreRepository.stopSnoreModule()

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