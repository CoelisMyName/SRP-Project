package com.scut

import android.app.Application
import android.icu.text.SimpleDateFormat
import androidx.lifecycle.viewModelScope
import com.scut.model.SleepRecord
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
import java.util.*

class DebugViewModel(application: Application) : BaseViewModel(application) {
    private val mDateFormatter = SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.getDefault())

    private var mLog = ""

    private var mLogFlow = MutableStateFlow(mLog)

    init {
        viewModelScope.launch(Dispatchers.Main) {
            SnoreRepository.getSnoreFlow().transform {
                val current = Date()
                val msg = when (it) {
                    is SnoreRepository.Message.Start -> {
                        val date = Date(it.timestamp)
                        "${mDateFormatter.format(current)}: start at ${mDateFormatter.format(date)}"
                    }
                    is SnoreRepository.Message.Package -> {
                        val snore = it.data
                        val date = Date(snore.timestamp)
                        "${mDateFormatter.format(current)}: voice start at ${
                            mDateFormatter.format(
                                date
                            )
                        }, length ${snore.length}ms, result is ${snore.confirm}"
                    }
                    is SnoreRepository.Message.Stop -> {
                        val date = Date(it.timestamp)
                        "${mDateFormatter.format(current)}: stop at ${mDateFormatter.format(date)}"
                    }
                    else -> {
                        ""
                    }
                }
                mLog = "$mLog\n$msg"
                emit(mLog)
            }.onEach {
                mLogFlow.emit(mLog)
            }.collect()
        }
    }

    fun startSnoreModule() = SnoreRepository.startSnoreModule()

    fun stopSnoreModule() = SnoreRepository.stopSnoreModule()

    fun getSnoreFlow() = SnoreRepository.getSnoreFlow()

    fun getSPLFlow() = SnoreRepository.getSPLFlow()

    fun getLOGFlow(): StateFlow<String> = mLogFlow

    fun newRender(type: String) = SnoreRepository.newRender(type)

    fun deleteSleepRecord(sl: SleepRecord) = SnoreRepository.deleteSleepRecord(sl)

    fun queryAllSleepRecord() = SnoreRepository.queryAllSleepRecord()

    fun querySleepRecordByTimestamp(timestamp: Long) =
        SnoreRepository.querySleepRecordByTimestamp(timestamp)
}