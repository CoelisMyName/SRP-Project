package com.scut

import android.content.Intent
import android.os.Bundle

import androidx.appcompat.app.AppCompatActivity
import com.scut.databinding.ActivityMainBinding
import com.scut.utils.MyMLLiveModel

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    val m_model: MyMLLiveModel = MyMLLiveModel

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        if (DEBUG) {
            val intent = Intent(this, DebugActivity::class.java)
            startActivity(intent)
        }
    }
}