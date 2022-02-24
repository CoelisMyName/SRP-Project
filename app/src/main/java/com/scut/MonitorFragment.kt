package com.scut

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.scut.databinding.FragmentMonitorBinding

class MonitorFragment : Fragment() {

    private lateinit var mBinding: FragmentMonitorBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        mBinding = FragmentMonitorBinding.inflate(inflater, container, false)
        return mBinding.root
    }
}