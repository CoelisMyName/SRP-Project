package com.scut.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentActivity
import androidx.viewpager2.adapter.FragmentStateAdapter
import androidx.viewpager2.widget.ViewPager2
import com.google.android.material.tabs.TabLayout
import com.scut.MainActivity
import com.scut.R
import com.scut.databinding.FragmentMainBinding

class MainFragment : Fragment() {
    private lateinit var mBinding: FragmentMainBinding
    // 两个子页面（根页面）
    private lateinit var mFMonitor: MonitorFragment
    private lateinit var mFRecord: RecordFragment
    // 导航用资源
    private lateinit var mFragments: Array<Fragment>
    private lateinit var mNavigatorText: Array<String>



    class SlideFragmentMonitor: FragmentStateAdapter {
        private var mLists = mutableListOf<Fragment>()
        constructor (fragment: Fragment): super (fragment)
        override fun createFragment(position: Int): Fragment {
            return this.mLists[position]
        }

        override fun getItemCount(): Int {
            return this.mLists.size
        }

        fun addFragment(fragment: Fragment) {
            this.mLists.add(fragment)
        }
    }

        override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
        ): View {
            mBinding = FragmentMainBinding.inflate(inflater, container, false)

            // 初始化导航用资源
            this.mNavigatorText = arrayOf(
                    getString(R.string.navigator_to_monitor),
                    getString(R.string.navigator_history))
            // 初始化页面
            this.mFMonitor = MonitorFragment()
            this.mFRecord = RecordFragment()
            this.mFragments = arrayOf(this.mFMonitor, this.mFRecord)

            // 添加页面及绑定
            val adp = SlideFragmentMonitor(this)
            adp.addFragment(this.mFMonitor)
            adp.addFragment(this.mFRecord)
            this.mBinding.mainView.adapter = adp

            // 设置监听
            this.mBinding.navigatorTab.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener{
                override fun onTabReselected(tab: TabLayout.Tab?) {}
                override fun onTabUnselected(tab: TabLayout.Tab?) {}
                override fun onTabSelected(tab: TabLayout.Tab?) {
                    mBinding.mainView.setCurrentItem(tab!!.position, true)
                }
            })

            this.mBinding.mainView.registerOnPageChangeCallback(object : ViewPager2.OnPageChangeCallback() {
                override fun onPageSelected(position: Int) {
                    super.onPageSelected(position)
                    mBinding.navigatorTab.getTabAt(position)?.select()
                }
            })

            // 选中首页
            this.mBinding.navigatorTab.getTabAt(0)?.select()

        return mBinding.root
    }
    public fun updateRecording(state: Boolean) {
        val s = when (state) {
            false -> getString(R.string.navigator_to_monitor)
            true -> getString(R.string.navigator_monitoring)
        }
        this.mBinding.navigatorTab.getTabAt(0)?.text = s
    }
//
//    private fun onClick(view: View) {
//        val activity = requireActivity() as MainActivity
//        if (view == mBinding.cardMonitor) {
//            activity.toFragment(MonitorFragment())
//        } else if (view == mBinding.cardReport) {
//            activity.toFragment(RecordFragment())
//        }
//    }
}