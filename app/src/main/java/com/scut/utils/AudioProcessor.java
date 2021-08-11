package com.scut.utils;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import androidx.annotation.NonNull;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioProcessor {
    public static final int SIZE = 60 * AudioRecorder.SAMPLE_RATE * AudioRecorder.BIT_DEPTH / 8;
    public static final int AUDIO_DATA = 0x800000;
    Handler handler = new Handler(Looper.getMainLooper(), AudioProcessor.this::handleMessage);
    ExecutorService service = Executors.newSingleThreadExecutor();
    AudioRecorder recorder = new AudioRecorder();
    byte[] bufferA = new byte[SIZE];
    byte[] bufferB = new byte[SIZE];
    int bufferLength = 0;

    public AudioProcessor() {
        recorder.setCallback(this::update);
    }

    public boolean handleMessage(@NonNull Message msg) {
        switch (msg.what) {
            case AUDIO_DATA:
                byte[] buffer = (byte[]) msg.obj;

                return true;
        }
        return false;
    }

    /**
     * @param data   音频数据
     * @param length 数据长度，必须小于60 * 44100 * 2
     */
    public void update(byte[] data, int length) {
        int copy = Math.min(bufferA.length - bufferLength, length);
        if (copy >= 0) {
            System.arraycopy(data, 0, bufferA, bufferLength, copy);
        }
        if (bufferA.length == bufferLength) {
            Message msg = handler.obtainMessage(AUDIO_DATA, bufferA);
            handler.sendMessage(msg);
            bufferLength = 0;
            swapBuffer();
        }
        if (length - copy > 0) {
            System.arraycopy(data, copy, bufferA, 0, length - copy);
            bufferLength = length - copy;
        }
    }

    /**
     * 交换缓冲区
     */
    void swapBuffer() {
        byte[] temp = bufferA;
        bufferA = bufferB;
        bufferB = temp;
    }
}
