package com.scut.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import com.scut.R
import com.scut.SnoreRepository
import com.scut.component.RenderFactory
import com.scut.databinding.FragmentMonitorBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.onEach
import kotlinx.coroutines.withContext

class MonitorFragment : Fragment() {

    private lateinit var mBinding: FragmentMonitorBinding
    private lateinit var mViewModel: MonitorViewModel
    private var mAudioRecordState = SnoreRepository.AudioRecordState.AudioRecordIDLE

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
        lifecycleScope.launchWhenResumed {
            mViewModel.getAudioRecordStateFlow().onEach {
                withContext(Dispatchers.Main) {
                    mAudioRecordState = it
                    if (it == SnoreRepository.AudioRecordState.AudioRecordIDLE || it == SnoreRepository.AudioRecordState.AudioRecordSTOP) {
                        mBinding.begin.setImageResource(R.drawable.ic_baseline_play_arrow_24)
                    }
                    if (it == SnoreRepository.AudioRecordState.AudioRecordSTART) {
                        mBinding.begin.setImageResource(R.drawable.ic_baseline_stop_24)
                    }
                    if (it == SnoreRepository.AudioRecordState.AudioRecordIDLE || it == SnoreRepository.AudioRecordState.AudioRecordSTART) {
                        mBinding.showDetail.visibility = View.GONE
                    }
                    if (it == SnoreRepository.AudioRecordState.AudioRecordSTOP) {
                        mBinding.showDetail.visibility = View.VISIBLE
                    }
                }
            }.collect()
            mViewModel.getDurationFlow().onEach {
                var seconds = it / 1000
                var minutes = seconds / 60
                val hours = minutes / 60
                seconds %= 60
                minutes %= 60
                val time = String.format("%02d:%02d:%02d", hours, minutes, seconds)
                withContext(Dispatchers.Main) {
                    mBinding.time.text = time
                }
            }.collect()
        }
        return mBinding.root
    }

    private fun onClick(view: View) {
        if (view == mBinding.begin) {
            if (mAudioRecordState == SnoreRepository.AudioRecordState.AudioRecordIDLE || mAudioRecordState == SnoreRepository.AudioRecordState.AudioRecordSTOP) {
                mViewModel.startSnoreModule()
            }
            if (mAudioRecordState == SnoreRepository.AudioRecordState.AudioRecordSTART) {
                mViewModel.stopSnoreModule()
            }
        }
        if (view == mBinding.showDetail) {
            //TODO show detail
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        if (mAudioRecordState == SnoreRepository.AudioRecordState.AudioRecordIDLE || mAudioRecordState == SnoreRepository.AudioRecordState.AudioRecordSTOP) {
            mViewModel.resetAudioRecordStateFlow()
        }
    }
}