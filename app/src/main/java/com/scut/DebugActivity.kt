package com.scut

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.scut.component.RenderFactory
import com.scut.databinding.ActivityDebugBinding
import com.scut.utils.ModuleController


class DebugActivity : AppCompatActivity(), View.OnClickListener {

    companion object {
        const val TAG = "DebugActivity"
    }

    private lateinit var binding: ActivityDebugBinding

    private val mController = ModuleController

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityDebugBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.start.setOnClickListener(this)
        binding.stop.setOnClickListener(this)
        mController.create()
        binding.textureView.setRender(RenderFactory.WAVE_RENDER)
        mController.registerNativeCallback(binding.textureView.getRender().getNativePointer())
        Log.d(TAG, "onSurfaceCreate: ")
    }

    override fun onStart() {
        super.onStart()
        Log.d(TAG, "onAudioDataStart: ")
        binding.textureView.onStart()
    }

    override fun onResume() {
        super.onResume()
        Log.d(TAG, "onResume: ")
        binding.textureView.onResume()
    }

    override fun onPause() {
        super.onPause()
        Log.d(TAG, "onPause: ")
        binding.textureView.onPause()
    }

    override fun onStop() {
        super.onStop()
        Log.d(TAG, "onAudioDataStop: ")
        binding.textureView.onStop()
    }


    override fun onClick(v: View?) {
        if (v == binding.start) {
            when (PackageManager.PERMISSION_GRANTED) {
                ContextCompat.checkSelfPermission(
                    this,
                    Manifest.permission.RECORD_AUDIO
                ) -> {
                    mController.start()
                }
                else -> {
                    requestPermissions(arrayOf(Manifest.permission.RECORD_AUDIO), 1)
                }
            }
        }
        if (v == binding.stop) {
            mController.stop()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        mController.stop()
        mController.unregisterNativeCallback(binding.textureView.getRender().getNativePointer())
        mController.destroy()
        Log.d(TAG, "onSurfaceDestroy: ")
    }
}