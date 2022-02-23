package com.scut

import androidx.room.Database
import androidx.room.RoomDatabase
import com.scut.utils.SleepRecord
import com.scut.utils.Snore

@Database(entities = [SleepRecord::class, Snore::class], version = 1)
abstract class SnoreDatabase : RoomDatabase() {
}