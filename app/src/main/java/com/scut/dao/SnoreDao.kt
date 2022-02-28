package com.scut.dao

import androidx.room.*
import com.scut.model.SleepRecord
import com.scut.model.SnoreRecord
import kotlinx.coroutines.flow.Flow

@Dao
abstract class SnoreDao {
    @Insert
    abstract suspend fun insertSleepRecord(vararg sl: SleepRecord): Array<Long>

    @Transaction
    @Query("UPDATE SleepRecord SET duration = :duration WHERE timestamp LIKE :timestamp")
    abstract suspend fun updateSleepRecordDuration(timestamp: Long, duration: Long): Int

    @Transaction
    @Query("UPDATE SleepRecord SET label = :label WHERE timestamp LIKE :timestamp")
    abstract suspend fun updateSleepRecordLabel(timestamp: Long, label: Double): Int

    @Delete
    abstract suspend fun deleteSleepRecordRow(vararg sl: SleepRecord): Int

    @Query("DELETE FROM SnoreRecord WHERE startTime = :startTime")
    abstract suspend fun deleteSnoreRecordRowByStartTime(startTime: Long): Int

    @Transaction
    open suspend fun deleteSleepRecord(vararg sl: SleepRecord): Int {
        var ret = deleteSleepRecordRow(*sl)
        for (s in sl) {
            ret += deleteSnoreRecordRowByStartTime(s.timestamp)
        }
        return ret
    }

    @Insert
    abstract suspend fun insertSnoreRecord(vararg sr: SnoreRecord): Array<Long>

    @Transaction
    @Query("SELECT * FROM SleepRecord")
    abstract fun queryAllSleepRecord(): Flow<List<SleepRecord>>

    @Transaction
    @Query("SELECT * FROM SleepRecord WHERE timestamp LIKE :timestamp")
    abstract fun querySleepRecordByTimestamp(timestamp: Long): Flow<List<SleepRecord>>

    @Transaction
    @Query("SELECT * FROM SnoreRecord WHERE startTime LIKE :startTime")
    abstract fun querySnoreRecordByStartTime(startTime: Long): Flow<List<SnoreRecord>>
}