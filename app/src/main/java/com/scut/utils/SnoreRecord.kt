package com.scut.utils

import androidx.room.Entity
import androidx.room.Ignore
import androidx.room.PrimaryKey

@Entity
data class SnoreRecord(
    @PrimaryKey val timestamp: Long,
    val description: String,
    @Ignore val snores: List<Snore>
)
