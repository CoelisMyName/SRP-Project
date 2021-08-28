package com.scut;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.scut.databinding.ActivityMainBinding;
import com.scut.utils.Utils;

import java.io.File;
import java.nio.ByteBuffer;
import java.nio.ShortBuffer;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        com.scut.databinding.ActivityMainBinding binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
    }

    @Override
    protected void onResume() {
        super.onResume();
        File dir = getExternalCacheDir();
        String filename = new File(dir, "test.txt").getAbsolutePath();
        Log.d(TAG, "onResume: " + filename);
    }
}