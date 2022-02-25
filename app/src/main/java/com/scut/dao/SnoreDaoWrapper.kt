package com.scut.dao

import com.scut.model.SleepRecord
import com.scut.model.SnoreRecord

class SnoreDaoWrapper(dao: SnoreDao) {
    private val mDao = dao

    suspend fun insert(vararg sl: SleepRecord) = mDao.insert(*sl)

    suspend fun update(vararg sl: SleepRecord) = mDao.update(*sl)

    suspend fun delete(vararg sl: SleepRecord) = mDao.delete(*sl)

    suspend fun insert(vararg sr: SnoreRecord) = mDao.insert(*sr)

    fun query() = mDao.query()

    fun query(timestamp: Long) = mDao.query(timestamp)
}