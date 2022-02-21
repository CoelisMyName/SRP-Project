package com.scut.utils

import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity

class PermissionManager(
    activity: AppCompatActivity,
    permissions: Array<String>,
    onGranted: () -> Unit,
    onDenied: (Array<String>) -> Unit
) {
    private val mPermissionLauncher: ActivityResultLauncher<Array<String>> =
        activity.registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) {
            callback(it)
        }

    private val mActivity = activity

    private val mOnGranted = onGranted

    private val mOnDenied = onDenied

    private val mPermissions = permissions

    private fun callback(it: Map<String, Boolean>) {
        val denied = mutableListOf<String>()
        for ((k, v) in it) {
            if (!v) {
                denied += k
            }
        }
        if (denied.isEmpty()) {
            mOnGranted()
        } else {
            mOnDenied(denied.toTypedArray())
        }
    }

    fun proceed() {
        val denied = Utils.deniedPermissions(mActivity, mPermissions)
        if (denied.isEmpty()) {
            mOnGranted()
        } else {
            mPermissionLauncher.launch(denied)
        }
    }
}