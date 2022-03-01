package com.scut

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.scut.databinding.FragmentMainBinding

class MainFragment : Fragment() {

    private lateinit var mBinding: FragmentMainBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        mBinding = FragmentMainBinding.inflate(inflater, container, false)
        mBinding.cardMonitor.setOnClickListener { onClick(it) }
        mBinding.cardReport.setOnClickListener { onClick(it) }
        return mBinding.root
    }

    private fun onClick(view: View) {
        if (view == mBinding.cardMonitor) {
            var activity = requireActivity() as MainActivity
            activity.toFragment(MonitorFragment())

        } else if (view == mBinding.cardReport) {
            var activity = requireActivity() as MainActivity
            activity.toFragment(ReportFragment())
        }
    }
}