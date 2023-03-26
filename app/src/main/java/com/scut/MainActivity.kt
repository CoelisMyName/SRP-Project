package com.scut

import android.content.Intent
import android.content.res.Configuration
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentManager
import androidx.lifecycle.ViewModelProvider
import com.scut.Config.DEBUG
import com.scut.databinding.ActivityMainBinding
import com.scut.ui.MainFragment
import com.scut.utils.PermissionManager


class MainActivity : AppCompatActivity() {
    private lateinit var mBinding: ActivityMainBinding
    private lateinit var mFragmentManger: FragmentManager
    private lateinit var mMainFragment: MainFragment
    private lateinit var mViewModel: MainViewModel
    val mPermissionManager: PermissionManager = PermissionManager(this)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        mBinding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(mBinding.root)
        mViewModel = ViewModelProvider(this)[MainViewModel::class.java]
        if (DEBUG) {
            val intent = Intent(this, DebugActivity::class.java)
            startActivity(intent)
        }

        mFragmentManger = supportFragmentManager
        val fragment = mFragmentManger.getFragment(mViewModel.mBundle, "MainFragment")
        if (fragment == null) {
            mMainFragment = MainFragment()
            mFragmentManger.beginTransaction().add(mBinding.container.id, mMainFragment).commit()
            mFragmentManger.putFragment(mViewModel.mBundle, "MainFragment", mMainFragment)
        } else {
            mMainFragment = fragment as MainFragment
        }

    }

    fun toFragment(fragment: Fragment) {
        mFragmentManger.beginTransaction().replace(mBinding.container.id, fragment)
            .addToBackStack(null).commit()
    }

    fun updateRecording (state: Boolean) {
        this.mMainFragment.updateRecording(state)
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        for (f in this.mFragmentManger.fragments) {
            this.mFragmentManger.beginTransaction().detach(f).commit()
            this.mFragmentManger.beginTransaction().attach(f).commit()
        }
    }
}