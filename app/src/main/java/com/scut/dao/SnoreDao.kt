package com.scut.dao

import androidx.room.Dao
import androidx.room.Insert
import androidx.room.Update
import com.scut.utils.SleepRecord

@Dao
abstract class SnoreDao {
    @Insert
    abstract fun insert(sr: SleepRecord): Int

    @Update
    abstract fun update(sr: SleepRecord): Int
}