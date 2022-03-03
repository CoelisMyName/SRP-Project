package com.scut.ui

import android.Manifest
import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import com.scut.MainActivity
import com.scut.MyService
import com.scut.R
import com.scut.SnoreRepository
import com.scut.component.RenderFactory
import com.scut.databinding.FragmentMonitorBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.withContext

class MonitorFragment : Fragment() {
    private var mAudioStart: SnoreRepository.Message<Long> = SnoreRepository.Message.Void()
    private var mAudioState: SnoreRepository.Message<Long> = SnoreRepository.Message.Void()
    private lateinit var mBinding: FragmentMonitorBinding
    private lateinit var mViewModel: MonitorViewModel
    private val mPermissions = arrayOf(
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Manifest.permission.RECORD_AUDIO
    )

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        mBinding = FragmentMonitorBinding.inflate(inflater, container, false)
        mViewModel = ViewModelProvider(this)[MonitorViewModel::class.java]
        mBinding.wave.setRender(mViewModel.newRender(RenderFactory.WAVE_RENDER))
        mBinding.begin.setOnClickListener { onClick(it) }
        mBinding.showDetail.setOnClickListener { onClick(it) }
        mBinding.showDetail.visibility = View.GONE
        initView()
        return mBinding.root
    }

    private fun initView() {
        lifecycleScope.launchWhenResumed {
            mViewModel.getAudioStateFlow().collectLatest {
                withContext(Dispatchers.Main) {
                    if (it is SnoreRepository.Message.Start) {
                        mAudioStart = it
                    }
                    if (it is SnoreRepository.Message.Stop) {
                        mBinding.begin.setImageResource(R.drawable.ic_baseline_play_arrow_24)
                    }
                    if (it is SnoreRepository.Message.Start) {
                        mBinding.begin.setImageResource(R.drawable.ic_baseline_stop_24)
                    }
                    mAudioState = it
                }
            }
        }
        lifecycleScope.launchWhenResumed {
            mViewModel.getDurationFlow().collectLatest {
                var seconds = it / 1000
                var minutes = seconds / 60
                val hours = minutes / 60
                seconds %= 60
                minutes %= 60
                val time = String.format("%02d:%02d:%02d", hours, minutes, seconds)
                withContext(Dispatchers.Main) {
                    mBinding.time.text = time
                }
            }
        }
        lifecycleScope.launchWhenResumed {
            mViewModel.getDetailVisibilityFlow().collectLatest {
                mBinding.showDetail.visibility = it
            }
        }
    }

    override fun onStart() {
        super.onStart()
        mBinding.wave.onStart()
    }

    override fun onResume() {
        super.onResume()
        mBinding.wave.onResume()
    }

    override fun onPause() {
        super.onPause()
        mBinding.wave.onPause()
    }

    override fun onStop() {
        super.onStop()
        mBinding.wave.onStop()
    }

    private fun startSnoreModule() {
        val intent = Intent(requireActivity() as MainActivity, MyService::class.java)
        ContextCompat.startForegroundService(requireActivity() as MainActivity, intent)
        mViewModel.startSnoreModule()
    }

    private fun stopSnoreModule() {
        //TODO fix restart foreground service crash
//        val service = Intent(requireActivity() as MainActivity, MyService::class.java)
//        (requireActivity() as MainActivity).stopService(service)
        mViewModel.stopSnoreModule()
    }

    private fun showPermissionDeniedMessage() {
        Toast.makeText(requireContext(), "Permission Denied", Toast.LENGTH_SHORT).show()
    }

    private fun onClick(view: View) {
        if (view == mBinding.begin) {
            if (mAudioState is SnoreRepository.Message.Stop || mAudioState is SnoreRepository.Message.Void) {
                val activity = requireActivity() as MainActivity
                activity.mPermissionManager.goCheckPermission(
                    mPermissions,
                    { startSnoreModule() },
                    { showPermissionDeniedMessage() })
            }
            if (mAudioState is SnoreRepository.Message.Start) {
                stopSnoreModule()
            }
        }
        if (view == mBinding.showDetail) {
            if (mAudioStart is SnoreRepository.Message.Start && mAudioState is SnoreRepository.Message.Stop) {
                val activity = requireActivity() as MainActivity
                val fragment = DetailFragment()
                val bundle = Bundle()
                bundle.putLong(
                    "timestamp",
                    (mAudioStart as SnoreRepository.Message.Start<Long>).timestamp
                )
                fragment.arguments = bundle
                activity.toFragment(fragment)
            }
        }
    }
}