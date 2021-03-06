package com.scut.model

import androidx.room.Entity
import androidx.room.PrimaryKey

@Entity
data class SnoreRecord(
    @PrimaryKey(autoGenerate = true)
    val id: Int = 0,
    val timestamp: Long = 0,
    val length: Long = 0,
    val confirm: Boolean = false,
    val startTime: Long = 0
)