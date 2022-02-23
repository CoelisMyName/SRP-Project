package com.scut.model

import androidx.room.Entity
import androidx.room.PrimaryKey

@Entity
data class SleepRecord(
    @PrimaryKey
    val timestamp: Long = 0,
    val description: String = "",
    val duration: Long = 0,
)
