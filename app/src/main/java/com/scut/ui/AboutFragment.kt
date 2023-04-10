package com.scut.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import androidx.fragment.app.Fragment
import com.scut.MainActivity
import com.scut.databinding.FragmentAboutBinding

/**
 * A simple [Fragment] subclass.
 * Provide APP about page and self-test entrance.
 * create an instance of this fragment.
 */
class AboutFragment : Fragment() {

    private lateinit var mBinding: FragmentAboutBinding

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?,
                              savedInstanceState: Bundle?): View {
        this.mBinding = FragmentAboutBinding.inflate(inflater, container, false)
        // 绑定返回按钮
        this.mBinding.goBack.setOnClickListener { (activity as MainActivity).popFragment() }
        // 绑定详情按钮
        this.mBinding.toOsa.setOnClickListener { toSubPage(it) }
        this.mBinding.toPsqi.setOnClickListener { toSubPage(it) }
        this.mBinding.toEss.setOnClickListener { toSubPage(it) }
        return this.mBinding.root
    }

    fun toSubPage (btn: View){
        val newFrag: Fragment? = when (btn) {
            this.mBinding.toOsa  -> AboutOsaFragment()
            this.mBinding.toPsqi -> PsqiFragment()
            this.mBinding.toEss  -> EssFragment()
            else -> null
        }
        if (newFrag != null) {
            (activity as MainActivity).toFragment(newFrag)
        }
    }
}