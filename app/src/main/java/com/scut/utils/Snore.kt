package com.scut.utils

import androidx.room.Entity
import androidx.room.PrimaryKey

/**
 * timestamp 音频段时间戳，length 音频段长度，单位毫秒，confirm 确认是鼾声，startTime 录音开始时间
 */
@Entity
data class Snore(
    @PrimaryKey val timestamp: Long = 0,
    val length: Long = 0,
    val confirm: Boolean = false,
    val startTime: Long = 0
)