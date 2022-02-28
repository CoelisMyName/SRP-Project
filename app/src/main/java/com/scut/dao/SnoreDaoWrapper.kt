package com.scut.dao

import com.scut.model.SleepRecord
import com.scut.model.SnoreRecord

class SnoreDaoWrapper(dao: SnoreDao) {
    private val mDao = dao

    suspend fun insertSleepRecord(vararg sl: SleepRecord) = mDao.insertSleepRecord(*sl)

    suspend fun deleteSleepRecord(vararg sl: SleepRecord) = mDao.deleteSleepRecord(*sl)

    suspend fun insertSnoreRecord(vararg sr: SnoreRecord) = mDao.insertSnoreRecord(*sr)

    fun queryAllSleepRecord() = mDao.queryAllSleepRecord()

    fun querySleepRecordByTimestamp(timestamp: Long) = mDao.querySleepRecordByTimestamp(timestamp)

    fun querySnoreRecordByStartTime(startTime: Long) = mDao.querySnoreRecordByStartTime(startTime)

    suspend fun updateSleepRecordDuration(timestamp: Long, duration: Long) =
        mDao.updateSleepRecordDuration(timestamp, duration)

    suspend fun updateSleepRecordLabel(timestamp: Long, label: Double) =
        mDao.updateSleepRecordLabel(timestamp, label)
}