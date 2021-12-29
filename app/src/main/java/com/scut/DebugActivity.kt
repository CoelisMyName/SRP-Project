package com.scut

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.scut.databinding.ActivityDebugBinding
import com.scut.utils.SnoringRecognition

const val TAG = "DebugActivity"

class DebugActivity : AppCompatActivity(), View.OnClickListener {

    private lateinit var binding: ActivityDebugBinding

    val m_snore: SnoringRecognition = SnoringRecognition

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityDebugBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.start.setOnClickListener(this)
        binding.stop.setOnClickListener(this)
//        m_model.m_clsCallback =  {minute, start, end, pos -> Log.d(TAG, "onCreate: ${minute}min ${start} ${end} ${pos}")}
//        m_model.m_splCallback = {spl -> Log.d(TAG, spl.toString()) }
    }

    override fun onClick(v: View?) {
        if (v == binding.start) {
            when {
                ContextCompat.checkSelfPermission(
                    this,
                    Manifest.permission.RECORD_AUDIO
                ) == PackageManager.PERMISSION_GRANTED -> {
                    m_snore.start()
                }
                else -> {
                    requestPermissions(arrayOf(Manifest.permission.RECORD_AUDIO), 1)
                }
            }
        }
        if (v == binding.stop) {
            m_snore.stop()
        }
    }
}