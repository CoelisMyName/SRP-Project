package com.scut

import android.app.Application
import androidx.lifecycle.AndroidViewModel

open class BaseViewModel(application: Application) : AndroidViewModel(application) {
    init {
        SnoreRepository.init(application)
    }
}