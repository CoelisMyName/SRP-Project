package com.scut

import android.annotation.SuppressLint
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.util.Log
import androidx.core.app.NotificationCompat

class MyService : Service() {
    companion object {
        const val CHANNEL_ID = "com.scut.MyChannel"
        const val CHANNEL_NAME = "MyChannel"
        const val TAG = "MyService"
    }

    private lateinit var mManager: NotificationManager
    private lateinit var mChannel: NotificationChannel

    @SuppressLint("ObsoleteSdkInt")
    override fun onCreate() {
        Log.d(TAG, "onCreate: ")
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            mManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            mChannel = mManager.getNotificationChannel(CHANNEL_ID) ?: NotificationChannel(
                CHANNEL_ID,
                CHANNEL_NAME,
                NotificationManager.IMPORTANCE_DEFAULT
            ).apply {
                mManager.createNotificationChannel(this)
            }
        }
    }

    override fun onBind(intent: Intent?): IBinder? {
        Log.d(TAG, "onBind: ")
        return null
    }

    override fun onDestroy() {
        Log.d(TAG, "onDestroy: ")
        super.onDestroy()
    }

    override fun onUnbind(intent: Intent?): Boolean {
        Log.d(TAG, "onUnbind: ")
        return super.onUnbind(intent)
    }

    override fun onRebind(intent: Intent?) {
        Log.d(TAG, "onRebind: ")
        super.onRebind(intent)
    }

    override fun onTaskRemoved(rootIntent: Intent?) {
        Log.d(TAG, "onTaskRemoved: ")
        super.onTaskRemoved(rootIntent)
    }

    @SuppressLint("UnspecifiedImmutableFlag")
    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Log.d(TAG, "onStartCommand: ")
        val toActivityIntent = Intent(this, MainActivity::class.java)
        val pendingIntent =
            PendingIntent.getActivity(this, 1000, toActivityIntent, 0)
        val notification = NotificationCompat.Builder(this, CHANNEL_ID).run {
            setContentText(getString(R.string.click_to_see_info))
            setContentTitle(getString(R.string.snoring_recognition))
            setSmallIcon(R.drawable.ic_launcher_foreground)
            setContentIntent(pendingIntent)
            build()
        }
        startForeground(1, notification)
        return START_NOT_STICKY
    }
}