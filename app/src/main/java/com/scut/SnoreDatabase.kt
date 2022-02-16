package com.scut

import androidx.room.Database
import androidx.room.RoomDatabase
import com.scut.utils.Snore
import com.scut.utils.SnoreRecord

@Database(entities = [SnoreRecord::class, Snore::class], version = 1)
abstract class SnoreDatabase : RoomDatabase() {
}