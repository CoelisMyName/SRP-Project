package com.scut

import android.content.Context
import androidx.room.Database
import androidx.room.Room
import androidx.room.RoomDatabase
import androidx.sqlite.db.SupportSQLiteDatabase
import com.scut.dao.SnoreDao
import com.scut.model.SleepRecord
import com.scut.model.SnoreRecord

@Database(entities = [SleepRecord::class, SnoreRecord::class], version = 1)
abstract class SnoreDatabase : RoomDatabase() {

    abstract fun getSnoreDao(): SnoreDao

    companion object {
        private val callback = object : RoomDatabase.Callback() {
            override fun onCreate(db: SupportSQLiteDatabase) {
                database?.run {
//                    //假数据
//                    val sleepRecord = SleepRecord(1646150831653L, "test", 60 * 60 * 1000L, 1.0)
//                    val snoreRecord = SnoreRecord(0, 1646150841653L, 1000, true, 1646150831653L)
//                    val dao = SnoreDaoWrapper(getSnoreDao())
//                    GlobalScope.launch {
//                        dao.insertSleepRecord(sleepRecord)
//                        dao.insertSnoreRecord(snoreRecord)
//                    }
                }
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