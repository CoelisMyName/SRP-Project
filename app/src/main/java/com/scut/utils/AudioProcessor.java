package com.scut.utils;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import androidx.annotation.NonNull;

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioProcessor {
    /**
     * 预留一定长度，保证降噪后的音频至少有60秒
     */
    public static final int SIZE = 65 * AudioRecorder.SAMPLE_RATE * AudioRecorder.BIT_DEPTH / 8;

    public static final int FRAME = 60 * AudioRecorder.SAMPLE_RATE * AudioRecorder.BIT_DEPTH / 8;

    public static final int AUDIO_DATA = 0x800000;

    Handler handler = new Handler(Looper.getMainLooper(), AudioProcessor.this::handleMessage);

    ExecutorService service = Executors.newSingleThreadExecutor();

    AudioRecorder recorder = new AudioRecorder();

    byte[] writeBuffer = new byte[SIZE];

    int writeLength = 0;

    byte[] readBuffer = new byte[SIZE];

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

    void write(byte[] buffer, int offset, int length) {
        int copy = Math.min(writeBuffer.length - writeLength, length);

        if (copy > 0) {
            System.arraycopy(buffer, offset, writeBuffer, writeLength, copy);
            writeLength += copy;
            offset += copy;
            length -= copy;
        }

        if(writeLength >= writeBuffer.length) {
            System.arraycopy(writeBuffer, FRAME, readBuffer, 0, SIZE - FRAME);
            writeLength = SIZE - FRAME;
            Message msg = handler.obtainMessage(AUDIO_DATA, writeBuffer);
            handler.sendMessage(msg);
            swapBuffer();
        }

        if(length > 0) {
            System.arraycopy(buffer, offset, writeBuffer, writeLength, length);
            writeLength += length;
        }

    }

    /**
     * @param data   音频数据
     * @param length 数据长度，必须小于60 * 44100 * 2
     */
    public void update(byte[] data, int length) {
        write(data, 0, length);

    }

    /**
     * 交换缓冲区
     */
    void swapBuffer() {
        byte[] temp = writeBuffer;
        writeBuffer = readBuffer;
        readBuffer = temp;
    }
}
