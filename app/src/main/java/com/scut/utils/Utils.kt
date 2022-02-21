package com.scut.utils

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import androidx.core.content.ContextCompat

object Utils {
    private fun permissionIsAlwaysBlocked(permission: String): Boolean {
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.P && permission == Manifest.permission.WRITE_EXTERNAL_STORAGE) return true
        return false
    }

    fun permissionGranted(context: Context, permissions: Array<String>): Boolean {
        for (permission in permissions) {
            val result = ContextCompat.checkSelfPermission(
                context,
                permission
            ) == PackageManager.PERMISSION_GRANTED
            if (result) continue
            if (permissionIsAlwaysBlocked(permission)) continue
            return false
        }
        return true
    }

    fun deniedPermissions(context: Context, permissions: Array<String>): Array<String> {
        val denied = mutableListOf<String>()
        for (permission in permissions) {
            val result = ContextCompat.checkSelfPermission(
                context,
                permission
            ) == PackageManager.PERMISSION_GRANTED
            if (result) continue
            if (permissionIsAlwaysBlocked(permission)) continue
            denied += permission
        }
        return denied.toTypedArray()
    }
}