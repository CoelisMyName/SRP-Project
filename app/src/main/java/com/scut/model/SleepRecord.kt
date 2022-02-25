package com.scut.model

import androidx.room.Entity
import androidx.room.PrimaryKey

@Entity
data class SleepRecord(
    @PrimaryKey
    val timestamp: Long = 0,
    var description: String = "",
    var duration: Long = 0,
)
