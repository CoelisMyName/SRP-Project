package com.scut.utils

import androidx.room.Entity
import androidx.room.Ignore
import androidx.room.PrimaryKey

@Entity
data class SleepRecord(
    @PrimaryKey
    val timestamp: Long = 0,
    val description: String = "",
    val duration: Long = 0,
    @Ignore
    val snores: List<SnoreRecord> = emptyList()
)
