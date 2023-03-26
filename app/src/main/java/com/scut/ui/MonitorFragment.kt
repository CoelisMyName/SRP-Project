package com.scut.ui

import android.Manifest
import android.content.Intent
import android.content.res.Configuration
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
    // 状态标记，用于明确提示文本
    private final var mIsFinish: Boolean = false

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        mBinding = FragmentMonitorBinding.inflate(inflater, container, false)
        mViewModel = ViewModelProvider(this)[MonitorViewModel::class.java]
        mBinding.wave.setRender(mViewModel.newRender(RenderFactory.WAVE_RENDER))
        mBinding.start.setOnClickListener { onClick(it) }
        mBinding.showDetail.setOnClickListener { onClick(it) }
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
//                        mBinding.start.setImageResource(R.drawable.ic_baseline_play_arrow_24)
                        if (mIsFinish) {
                            mBinding.start.text = getString(R.string.monitor_tap_to_restart)
                            mBinding.monitorNotes.text = getString(R.string.monitor_note_stop)
                        }
                        else {
                            mBinding.start.text = getString(R.string.monitor_tap_to_start)
                            mBinding.monitorNotes.text = getString(R.string.monitor_note_ready)
                        }
                    }
                    if (it is SnoreRepository.Message.Start) {
//                        mBinding.start.setImageResource(R.drawable.ic_baseline_stop_24)
                        mBinding.start.text = getString(R.string.monitor_tap_to_stop)
                        mBinding.monitorNotes.text = getString(R.string.monitor_note_recording)
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
                if (it == View.VISIBLE) {
                    mBinding.showDetail.isEnabled = true
                    mBinding.showDetail.text = getString(R.string.monitor_detail_ok)
                }
                else {
                    mBinding.showDetail.isEnabled = false
                    mBinding.showDetail.text = getString(R.string.monitor_detail_not)
                }
            }
        }
        this.mBinding.wave.onDarkModeChange(when (resources.configuration.uiMode and Configuration.UI_MODE_NIGHT_MASK) {
            Configuration.UI_MODE_NIGHT_YES -> true
            else -> false
        })
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
        // 更新标记
        this.mIsFinish = false
        // 更新显示内容
        (requireActivity() as MainActivity).updateRecording(true)
        mBinding.start.text = getString(R.string.monitor_tap_to_stop)
        mBinding.monitorNotes.text = getString(R.string.monitor_note_recording)
    }

    private fun stopSnoreModule() {
        val service = Intent(requireActivity() as MainActivity, MyService::class.java)
        (requireActivity() as MainActivity).stopService(service)
        mViewModel.stopSnoreModule()
        // 更新标记
        this.mIsFinish = true
        // 更新显示内容
        (requireActivity() as MainActivity).updateRecording(false)
        mBinding.start.text = getString(R.string.monitor_tap_try_stop)
        mBinding.monitorNotes.text = getString(R.string.monitor_note_stopping)
    }

    private fun showPermissionDeniedMessage() {
        Toast.makeText(requireContext(), "Permission Denied", Toast.LENGTH_SHORT).show()
    }

    private fun onClick(view: View) {
        if (view == mBinding.start) {
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