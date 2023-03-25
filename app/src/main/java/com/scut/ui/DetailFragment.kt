package com.scut.ui

import android.annotation.SuppressLint
import android.icu.text.SimpleDateFormat
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.LinearLayoutManager
import com.scut.R
import com.scut.databinding.FragmentDetailBinding
import com.scut.model.SleepRecord
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.flow.flowOn
import java.util.*

class DetailFragment : Fragment() {

    private lateinit var mBinding: FragmentDetailBinding
    private lateinit var mViewModel: DetailViewModel
    private lateinit var mAdapter: SnoreRecordAdapter


    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        mBinding = FragmentDetailBinding.inflate(inflater, container, false)
        mViewModel = ViewModelProvider(this)[DetailViewModel::class.java]
        mAdapter = SnoreRecordAdapter()
        mBinding.recycler.setHasFixedSize(true)
        val layoutManager = LinearLayoutManager(requireContext())
        mBinding.recycler.layoutManager = layoutManager
        mBinding.recycler.adapter = mAdapter
        initView()
        return mBinding.root
    }

    @SuppressLint("NotifyDataSetChanged")
    private fun initView() {
        arguments?.run {
            val timestamp = getLong("timestamp", 0L)
            if (timestamp <= 0) return@run
            lifecycleScope.launchWhenResumed {
                mViewModel.querySleepRecordByTimestamp(timestamp).flowOn(Dispatchers.Main)
                    .collectLatest {
                        if (it.isEmpty()) {
                            Toast.makeText(requireContext(), "找不到记录，可能被删除", Toast.LENGTH_LONG)
                                .show()
                            //TODO 弹出fragment
                        } else {
                            showSleepRecord(it[0])
                        }
                    }
            }
            lifecycleScope.launchWhenResumed {
                mViewModel.querySnoreRecordByStartTime(timestamp).flowOn(Dispatchers.Main)
                    .collectLatest {
                        mAdapter.mList = it
                        mAdapter.notifyDataSetChanged()
                    }
            }
        }
    }

    private fun smartFormatDuration(duration: Long): String {
        val byS     = duration / 1000L
        val onlyS   = byS % 60L
        val byM     = byS / 60L
        val onlyM   = byM % 60L
        if (byS < 60L) {
            return getString(R.string.duration_s, byS)
        }
        else if (byM < 60L) {
            return getString(R.string.duration_ms, byM, onlyS)
        }
        else {
            val byH = byM / 60L
            return getString(R.string.duration_hm, byH, onlyM)
        }
    }

    private fun showSleepRecord(sl: SleepRecord) {
        // 更新文字内容显示
        mBinding.sleepDate.text =
            SimpleDateFormat("yyyy/MM/dd HH:mm:ss", Locale.getDefault()).format(sl.timestamp)
        mBinding.sleepDuration.text = this.smartFormatDuration(sl.duration)
        mBinding.snoreTotalDuration.text = this.smartFormatDuration(sl.snoreTotalDuration)
        mBinding.snoreTimes.text = sl.snoreTimes.toString()
        mBinding.isPatient.text = when (sl.label) {
            0.0 -> getString(R.string.no)
            1.0 -> getString(R.string.yes)
            -1.0 -> getString(R.string.calculating)
            else -> getString(R.string.unexcept)
        }
        mBinding.subtitle.text = when (sl.label) {
            0.0 -> getString(R.string.detail_subtitle_healthy)
            1.0 -> getString(R.string.detail_subtitle_maybe)
            -1.0 -> getString(R.string.detail_subtitle_calcing)
            else -> getString(R.string.detail_subtitle_exception)
        }
        // 更新颜色，无鼾声无标签为正常，有标签为危险，其他为警告
        var txtColor = R.color.state_warning_bg
        var bgColor = R.color.state_warning
        if (sl.label == 1.0) {
            txtColor = R.color.state_danger_bg
            bgColor = R.color.state_danger
        }
        else if ((sl.label == 0.0) and (sl.snoreTimes == 0L)) {
            txtColor = R.color.state_healthy_bg
            bgColor = R.color.state_healthy
        }
        val context = mBinding.root.context
        val txtClr = context.getColor (txtColor)
        mBinding.sleepDate.setTextColor (txtClr)
        mBinding.subtitle.setTextColor (txtClr)
        val bgClr = context.getColor(bgColor)
        mBinding.resultHolder.setBackgroundColor(bgClr)
//        activity?.window?.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS)
//        activity?.window?.statusBarColor = bgClr
    }
}