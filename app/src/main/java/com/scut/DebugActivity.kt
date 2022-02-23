package com.scut

import android.Manifest
import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import com.scut.component.RenderFactory
import com.scut.databinding.ActivityDebugBinding
import com.scut.utils.PermissionManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.onEach
import kotlinx.coroutines.withContext


class DebugActivity : AppCompatActivity(), View.OnClickListener {
    companion object {
        const val TAG = "DebugActivity"
    }

    private lateinit var mBinding: ActivityDebugBinding

    private lateinit var mPermissionManager: PermissionManager

    private lateinit var mViewModel: DebugViewModel

    private var mLOGText = ""

    private val mPermissions = arrayOf(
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.RECORD_AUDIO
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        mBinding = ActivityDebugBinding.inflate(layoutInflater)
        setContentView(mBinding.root)
        mViewModel = ViewModelProvider(this)[DebugViewModel::class.java]
        mBinding.start.setOnClickListener(this)
        mBinding.stop.setOnClickListener(this)
        mPermissionManager = PermissionManager(
            this,
            mPermissions,
            { startSnoreModule() },
            { showPermissionDeniedMessage() })
        mBinding.textureView.setRender(mViewModel.newRender(RenderFactory.WAVE_RENDER))
        lifecycleScope.launchWhenResumed {
            mViewModel.getLOGFlow().onEach {
                mLOGText = mLOGText + "\n" + it
                setLogTextView(mLOGText)
            }.collect()
        }
    }

    private suspend fun setLogTextView(log: String) = withContext(Dispatchers.Main) {
        mBinding.log.text = log
    }

    override fun onStart() {
        super.onStart()
        mBinding.textureView.onStart()
    }

    override fun onResume() {
        super.onResume()
        mBinding.textureView.onResume()
    }

    override fun onPause() {
        super.onPause()
        mBinding.textureView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mBinding.textureView.onStop()
    }

    private fun showPermissionDeniedMessage() {
        Toast.makeText(this, "Permission Denied", Toast.LENGTH_SHORT).show()
    }

    private fun startSnoreModule() = mViewModel.startSnoreModule()

    private fun stopSnoreModule() = mViewModel.stopSnoreModule()

    override fun onClick(v: View?) {
        if (v == mBinding.start) {
            mPermissionManager.proceed()
        }
        if (v == mBinding.stop) {
            stopSnoreModule()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        mViewModel.stopSnoreModule()
    }
}