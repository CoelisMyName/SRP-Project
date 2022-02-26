package com.scut

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.scut.databinding.FragmentReportBinding

class ReportFragment : Fragment() {

    private lateinit var mBinding: FragmentReportBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        mBinding = FragmentReportBinding.inflate(inflater, container, false)
        return mBinding.root
    }
}