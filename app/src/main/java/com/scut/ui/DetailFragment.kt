package com.scut.ui

import android.annotation.SuppressLint
import android.icu.text.SimpleDateFormat
import android.icu.util.TimeZone
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.GridLayoutManager
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
        val layoutManager = GridLayoutManager(requireContext(), 2)
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

    private fun showSleepRecord(sl: SleepRecord) {
        mBinding.sleepDate.text = getString(
            R.string.sleep_test_date,
            SimpleDateFormat("yyyy/MM/dd HH:mm:ss", Locale.getDefault()).format(sl.timestamp)
        )
        mBinding.sleepDuration.text = getString(
            R.string.sleep_duration,
            SimpleDateFormat("HH:mm:ss", Locale.UK).apply { timeZone = TimeZone.getTimeZone("UTC") }
                .format(sl.duration)
        )
        mBinding.snoreTotalDuration.text = getString(
            R.string.snore_duration,
            SimpleDateFormat("HH:mm:ss", Locale.UK).apply { timeZone = TimeZone.getTimeZone("UTC") }
                .format(sl.snoreTotalDuration)
        )
        mBinding.snoreTimes.text = getString(R.string.snore_times, sl.snoreTimes)
        val isPatient = when (sl.label) {
            0.0 -> getString(R.string.no)
            1.0 -> getString(R.string.yes)
            -1.0 -> getString(R.string.calculating)
            else -> {
                ""
            }
        }
        mBinding.isPatient.text = getString(R.string.is_patient_or_not, isPatient)
    }
}