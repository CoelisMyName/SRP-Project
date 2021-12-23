package com.scut.utils

import java.text.DecimalFormat
import java.text.SimpleDateFormat
import java.util.*

data class SPL(
    val timestamp: Long = 0,
    val a_sum: Double = 0.0, val c_sum: Double = 0.0, val z_sum: Double = 0.0,
    val freq: DoubleArray = DoubleArray(8) { 0.0 }, val a_pow: DoubleArray = DoubleArray(8) { 0.0 },
    val c_pow: DoubleArray = DoubleArray(8) { 0.0 }, val z_pow: DoubleArray = DoubleArray(8) { 0.0 }
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as SPL

        if (timestamp != other.timestamp) return false
        if (a_sum != other.a_sum) return false
        if (c_sum != other.c_sum) return false
        if (z_sum != other.z_sum) return false
        if (!freq.contentEquals(other.freq)) return false
        if (!a_pow.contentEquals(other.a_pow)) return false
        if (!c_pow.contentEquals(other.c_pow)) return false
        if (!z_pow.contentEquals(other.z_pow)) return false

        return true
    }

    override fun hashCode(): Int {
        var result = timestamp.hashCode()
        result = 31 * result + a_sum.hashCode()
        result = 31 * result + c_sum.hashCode()
        result = 31 * result + z_sum.hashCode()
        result = 31 * result + freq.contentHashCode()
        result = 31 * result + a_pow.contentHashCode()
        result = 31 * result + c_pow.contentHashCode()
        result = 31 * result + z_pow.contentHashCode()
        return result
    }

    override fun toString(): String {
        val date = Date()
        val dateFormat = SimpleDateFormat("HH:mm:ss.SSS", Locale.CHINA)
        val decimalFormat = DecimalFormat("###0.00")
        return "${dateFormat.format(date)}\n" +
                "A计权： total = ${decimalFormat.format(a_sum)}dBA\n" +
                "C计权： total = ${decimalFormat.format(c_sum)}dBC\n" +
                "Z计权： total = ${decimalFormat.format(z_sum)}dBZ"
    }
}
