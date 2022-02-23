package com.scut.model

import androidx.room.Embedded
import androidx.room.Relation

data class SleepWithSnoreRecord(
    @Embedded
    val sleepRecord: SleepRecord,
    @Relation(
        parentColumn = "timestamp",
        entityColumn = "startTime",
    )
    val snoreRecord: List<SnoreRecord>,
)