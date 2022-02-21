package com.scut

import android.app.Application
import android.icu.text.SimpleDateFormat
import androidx.lifecycle.AndroidViewModel
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.transform
import java.util.*

class DebugViewModel(application: Application) : AndroidViewModel(application) {

    private val mSnoreRepository = SnoreRepository

    private val mDateFormatter = SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.getDefault())

    init {
        mSnoreRepository.init(application)
    }

    fun startSnoreModule() = mSnoreRepository.startSnoreModule()

    fun stopSnoreModule() = mSnoreRepository.stopSnoreModule()

    fun getSnoreFlow() = mSnoreRepository.getSnoreFlow()

    fun getSPLFlow() = mSnoreRepository.getSPLFlow()

    fun getLOGFlow(): Flow<String> {
        return mSnoreRepository.getSnoreFlow().transform {
            val current = Date()
            val msg = when (it) {
                is SnoreRepository.Message.Start -> {
                    val date = Date(it.timestamp)
                    "${mDateFormatter.format(current)}: start at ${mDateFormatter.format(date)}"
                }
                is SnoreRepository.Message.Package -> {
                    val snore = it.data
                    val date = Date(snore.timestamp)
                    "${mDateFormatter.format(current)}: voice start at ${mDateFormatter.format(date)}, length ${snore.length}ms, result is ${snore.confirm}"
                }
                is SnoreRepository.Message.Stop -> {
                    val date = Date(it.timestamp)
                    "${mDateFormatter.format(current)}: stop at ${mDateFormatter.format(date)}"
                }
                else -> {
                    ""
                }
            }
            emit(msg)
        }
    }

    fun newRender(type: String) = mSnoreRepository.newRender(type)
}