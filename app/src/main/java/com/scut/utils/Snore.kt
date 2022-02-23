package com.scut.utils

/**
 * timestamp 音频段时间戳，length 音频段长度，单位毫秒，confirm 确认是鼾声，startTime 录音开始时间
 */
data class Snore(
    val timestamp: Long = 0,
    val length: Long = 0,
    val confirm: Boolean = false,
    val startTime: Long = 0
)