package com.scut;

import android.app.Application;
import android.util.Log;

import com.scut.utils.Utils;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

public class MyApplication extends Application {
    private static final String TAG = "MyApplication";

    @Override
    public void onCreate() {
        super.onCreate();
        try {
            InputStream is = getAssets().open("test.wav");
            int size = (int) getAssets().openFd("test.wav").getLength();
            if (size > 0) {
                byte[] buffer = new byte[size];
                int read = is.read(buffer);
                Log.d(TAG, "onCreate: size = " + size + " read = " + read);
                String noise = new File(getExternalCacheDir(), "profile.txt").getAbsolutePath();
                Utils.generateNoiseProfile(buffer, 0, 1, noise);
                is.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    @Override
    public void onTerminate() {
        super.onTerminate();
    }
}
