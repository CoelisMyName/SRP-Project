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
import com.scut.databinding.FragmentRecordBinding
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
        mAdapter = SleepRecordAdapter()
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
}
