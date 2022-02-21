package com.scut.dao

import androidx.room.Dao
import androidx.room.Insert
import androidx.room.Update
import com.scut.utils.SnoreRecord

@Dao
abstract class SnoreDao {
    @Insert
    abstract fun insert(sr: SnoreRecord): Int

    @Update
    abstract fun update(sr: SnoreRecord): Int
}