package com.scut.utils

import android.content.Context
import java.io.BufferedWriter
import java.io.File
import java.io.FileOutputStream
import java.util.concurrent.PriorityBlockingQueue

object MyLogger {

    private val queue = PriorityBlockingQueue<Log>()

    private lateinit var bos: BufferedWriter

    private val thd = Thread {
        while (queue.isNotEmpty()) {
            val log = queue.poll()
        }
    }

    /**
     * Application
     */
    fun applicationCreate(context: Context) {
        val dir = File(context.externalCacheDir, "log")
        if (!dir.exists()) dir.mkdir()
        val file = File(dir, "log.txt")
        bos = FileOutputStream(file, true).bufferedWriter()
    }

    fun applicationTerminate() {
        bos.flush()
        bos.close()
    }

    private fun add(msg: String) {
        val curr = System.currentTimeMillis()
        queue.add(Log(curr, msg))
    }

    fun info(msg: String) {
        add("[INFO] $msg")
    }

    fun debug(msg: String) {
        add("[DEBUG] $msg")
    }

    fun warn(msg: String) {
        add("[WARN] $msg")
    }

    fun error(msg: String) {
        add("[ERROR] $msg")
    }

    data class Log(val time: Long, val msg: String)
}