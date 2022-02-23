package com.scut

import android.content.Context
import androidx.room.Database
import androidx.room.Room
import androidx.room.RoomDatabase
import androidx.sqlite.db.SupportSQLiteDatabase
import com.scut.dao.SnoreDao
import com.scut.model.SleepRecord
import com.scut.utils.Snore

@Database(entities = [SleepRecord::class, Snore::class], version = 1)
abstract class SnoreDatabase : RoomDatabase() {

    abstract fun getSnoreDao(): SnoreDao

    companion object {
        private val callback = object : RoomDatabase.Callback() {
            override fun onCreate(db: SupportSQLiteDatabase) {
                //TODO
            }
        }

        private var database: SnoreDatabase? = null

        @Synchronized
        fun getInstance(context: Context): SnoreDatabase {
            if (database != null) {
                return database!!
            }
            synchronized(this) {
                if (database != null) {
                    return database!!
                }
                val db = if (database != null) {
                    database
                } else {
                    Room.databaseBuilder(
                        context.applicationContext,
                        SnoreDatabase::class.java,
                        "snore.db"
                    ).run {
                        addCallback(callback)
                        build()
                    }
                } as SnoreDatabase
                database = db
                return db
            }
        }
    }
}