package com.scut.model

import androidx.room.Entity
import androidx.room.PrimaryKey

@Entity
data class SleepRecord(
    @PrimaryKey
    val timestamp: Long = 0,
    val description: String = "",
    var duration: Long = 0,
    val label: Double = -1.0,
    var snoreTimes: Long = 0,
    var snoreTotalDuration: Long = 0
)
