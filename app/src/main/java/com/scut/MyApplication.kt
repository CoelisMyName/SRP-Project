package com.scut

import android.app.Application
import com.scut.utils.LibSRP

class MyApplication : Application() {
    override fun onCreate() {
        super.onCreate()
        assets?.also { LibSRP.setAssetManager(assets) }
        SnoreDatabase.getInstance(this).openHelper.writableDatabase
    }
}