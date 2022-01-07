package com.scut.utils

data class SnoreConfig(
    var saveWhole: Boolean = false,
    var savePerMinute: Boolean = false,
    var saveSegment: Boolean = false,
    var savePositive: Boolean = false
)
