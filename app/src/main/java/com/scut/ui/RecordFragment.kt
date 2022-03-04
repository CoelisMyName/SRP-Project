package com.scut.ui

import android.annotation.SuppressLint
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import com.scut.MainActivity
import com.scut.databinding.FragmentRecordBinding
import com.scut.model.SleepRecord
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.onEach

class RecordFragment : Fragment() {
    private lateinit var mBinding: FragmentRecordBinding
    private lateinit var mAdapter: SleepRecordAdapter
    private lateinit var mViewModel: RecordViewModel

    @SuppressLint("NotifyDataSetChanged")
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        mBinding = FragmentRecordBinding.inflate(inflater, container, false)
        mViewModel = ViewModelProvider(this)[RecordViewModel::class.java]
        mBinding.recycler.setHasFixedSize(true)
        mAdapter = SleepRecordAdapter(object : SleepRecordAdapter.SleepRecordCallback {
            override fun showDetail(sl: SleepRecord) {
                showDetail(sl.timestamp)
            }
        })
        val layoutManager = LinearLayoutManager(requireContext())
        mBinding.recycler.layoutManager = layoutManager
        mBinding.recycler.adapter = mAdapter
        lifecycleScope.launchWhenResumed {
            mViewModel.queryAllSleepRecord().onEach {
                mAdapter.mList = it
                mAdapter.notifyDataSetChanged()
            }.collect()
        }
        return mBinding.root
    }

    fun showDetail(timestamp: Long) {
        val activity = requireActivity() as MainActivity
        val fragment = DetailFragment()
        val bundle = Bundle()
        bundle.putLong("timestamp", timestamp)
        fragment.arguments = bundle
        activity.toFragment(fragment)
    }
}
