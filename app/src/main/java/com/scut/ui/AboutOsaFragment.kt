package com.scut.ui

import android.os.Bundle
import android.text.method.LinkMovementMethod
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import com.scut.MainActivity
import com.scut.databinding.FragmentAboutOsaBinding

/**
 * A simple [Fragment] subclass.
 * Use the [AboutOsaFragment.newInstance] factory method to
 * create an instance of OSA information fragment.
 */
class AboutOsaFragment : Fragment() {

    private lateinit var mBinding: FragmentAboutOsaBinding

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?,
                              savedInstanceState: Bundle?): View {
        this.mBinding = FragmentAboutOsaBinding.inflate(inflater, container, false)
        // 绑定返回按钮
        this.mBinding.goBack.setOnClickListener { (activity as MainActivity).popFragment() }
        // 允许超链接跳转
        this.mBinding.gotoRef.movementMethod = LinkMovementMethod.getInstance()
        return this.mBinding.root
    }
}