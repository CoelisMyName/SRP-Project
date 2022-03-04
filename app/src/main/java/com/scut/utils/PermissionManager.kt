package com.scut.utils

import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts
import androidx.fragment.app.FragmentActivity

class PermissionManager(activity: FragmentActivity) {
    private val mPermissionLauncher: ActivityResultLauncher<Array<String>> =
        activity.registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) {
            checkResult(it)
        }

    private val mActivity = activity

    private var mOnGranted: () -> Unit = {}

    private var mOnDenied: (Array<String>) -> Unit = {}

    fun goCheckPermission(
        permissions: Array<String>,
        onGranted: () -> Unit = {},
        onDenied: (Array<String>) -> Unit = {}
    ) {
        mOnGranted = onGranted
        mOnDenied = onDenied
        val denied = Utils.deniedPermissions(mActivity, permissions)
        if (denied.isEmpty()) {
            onGranted()
        } else {
            mPermissionLauncher.launch(denied)
        }
    }

    private fun checkResult(it: Map<String, Boolean>) {
        val denied = mutableListOf<String>()
        for ((k, v) in it) {
            if (!v) {
                denied += k
            }
        }
        if (denied.isEmpty()) {
            onGranted()
        } else {
            onDenied(denied.toTypedArray())
        }
    }

    private fun onGranted() {
        mOnGranted()
        mOnGranted = {}
        mOnDenied = {}
    }

    private fun onDenied(denied: Array<String>) {
        mOnDenied(denied)
        mOnGranted = {}
        mOnDenied = {}
    }
}