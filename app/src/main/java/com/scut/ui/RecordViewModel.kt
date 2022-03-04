package com.scut.ui

import android.app.Application
import com.scut.BaseViewModel
import com.scut.SnoreRepository

class RecordViewModel(application: Application) : BaseViewModel(application) {
    fun queryAllSleepRecord() = SnoreRepository.queryAllSleepRecord()
}