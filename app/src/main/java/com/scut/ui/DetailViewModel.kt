package com.scut.ui

import android.app.Application
import com.scut.BaseViewModel
import com.scut.SnoreRepository

class DetailViewModel(application: Application) : BaseViewModel(application) {

    fun querySleepRecordByTimestamp(timestamp: Long) =
        SnoreRepository.querySleepRecordByTimestamp(timestamp)

    fun querySnoreRecordByStartTime(startTime: Long) =
        SnoreRepository.querySnoreRecordByStartTime(startTime)
}