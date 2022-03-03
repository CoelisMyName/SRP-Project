package com.scut.ui

import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
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

    private lateinit var mBinding: FragmentMonitorBinding
    private lateinit var mViewModel: MonitorViewModel
    private var mAudioStart: SnoreRepository.Message<Long> = SnoreRepository.Message.Void()
    private var mAudioState: SnoreRepository.Message<Long> = SnoreRepository.Message.Void()

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

    private fun onClick(view: View) {
        if (view == mBinding.begin) {
            if (mAudioState is SnoreRepository.Message.Stop || mAudioState is SnoreRepository.Message.Void) {
                mViewModel.startSnoreModule()
                val intent = Intent(requireContext(), MyService::class.java)
                ContextCompat.startForegroundService(requireContext(), intent)
            }
            if (mAudioStart is SnoreRepository.Message.Start) {
                val service = Intent(requireActivity(), MyService::class.java)
                requireActivity().stopService(service)
                mViewModel.stopSnoreModule()
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