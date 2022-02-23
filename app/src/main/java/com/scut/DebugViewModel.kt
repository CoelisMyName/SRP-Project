package com.scut

import android.app.Application
import android.icu.text.SimpleDateFormat
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.scut.model.SleepRecord
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
import java.util.*

class DebugViewModel(application: Application) : AndroidViewModel(application) {

    private val mSnoreRepository = SnoreRepository

    private val mDateFormatter = SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.getDefault())

    private var mLog = ""

    private var mLogFlow = MutableStateFlow(mLog)

    init {
        mSnoreRepository.init(application)
        viewModelScope.launch(Dispatchers.Main) {
            mSnoreRepository.getSnoreFlow().transform {
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

    fun startSnoreModule() = mSnoreRepository.startSnoreModule()

    fun stopSnoreModule() = mSnoreRepository.stopSnoreModule()

    fun getSnoreFlow() = mSnoreRepository.getSnoreFlow()

    fun getSPLFlow() = mSnoreRepository.getSPLFlow()

    fun getLOGFlow(): StateFlow<String> = mLogFlow

    fun newRender(type: String) = mSnoreRepository.newRender(type)

    fun deleteSleepRecord(sl: SleepRecord) = SnoreRepository.deleteSleepRecord(sl)

    fun query() = SnoreRepository.query()

    fun query(timestamp: Long) = SnoreRepository.query(timestamp)
}