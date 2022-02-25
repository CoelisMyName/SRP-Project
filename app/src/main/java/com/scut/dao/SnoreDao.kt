package com.scut.dao

import androidx.room.*
import com.scut.model.SleepRecord
import com.scut.model.SleepWithSnoreRecord
import com.scut.model.SnoreRecord
import kotlinx.coroutines.flow.Flow

@Dao
abstract class SnoreDao {
    @Insert
    abstract suspend fun insert(vararg sl: SleepRecord): Array<Long>

    @Update
    abstract suspend fun update(vararg sl: SleepRecord): Int

    @Delete
    abstract suspend fun deleteSleepRecord(vararg sl: SleepRecord): Int

    @Query("DELETE FROM SnoreRecord WHERE startTime = :startTime")
    abstract suspend fun deleteSnoreRecordByStartTime(startTime: Long): Int

    @Transaction
    open suspend fun delete(vararg sl: SleepRecord): Int {
        var ret = deleteSleepRecord(*sl)
        for (s in sl) {
            ret += deleteSnoreRecordByStartTime(s.timestamp)
        }
        return ret
    }

    @Insert
    abstract suspend fun insert(vararg sr: SnoreRecord): Array<Long>

    @Transaction
    @Query("SELECT * FROM SleepRecord")
    abstract fun query(): Flow<List<SleepWithSnoreRecord>>

    @Transaction
    @Query("SELECT * FROM SleepRecord WHERE timestamp IN (:timestamp)")
    abstract fun query(vararg timestamp: Long): Flow<List<SleepWithSnoreRecord>>
}