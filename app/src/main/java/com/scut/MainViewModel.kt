package com.scut

import android.app.Application
import android.os.Bundle
import androidx.lifecycle.AndroidViewModel

class MainViewModel(application: Application) : AndroidViewModel(application) {
    private val mRepository = SnoreRepository
    val mBundle = Bundle()

    init {
        mRepository.init(application)
    }


}