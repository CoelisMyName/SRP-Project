package com.scut

import android.app.Application
import androidx.lifecycle.AndroidViewModel

class MainViewModel(application: Application) : AndroidViewModel(application) {
    private val mRepository = SnoreRepository

    init {
        mRepository.init(application)
    }


}