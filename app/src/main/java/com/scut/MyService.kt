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
import androidx.core.app.NotificationCompat

class MyService : Service() {

    private val CHANNEL_ID = "com.scut.MyService"
    private val CHANNEL_NAME = "com.scut.name"

    private lateinit var mManager: NotificationManager
    private lateinit var mChannel: NotificationChannel

    @SuppressLint("ObsoleteSdkInt")
    override fun onCreate() {
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
        return null
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        val intent = Intent(this, DebugActivity::class.java)
        val pendingIntent = PendingIntent.getActivity(this, 0, intent, 0)
        val notification = NotificationCompat.Builder(this, CHANNEL_ID).run {
            setContentText("this is text")
            setContentTitle("this is title")
            setSmallIcon(R.drawable.ic_launcher_foreground)
            setContentIntent(pendingIntent)
            build()
        }
        startForeground(1, notification)
        return START_NOT_STICKY
    }
}