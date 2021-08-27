package com.scut.utils;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import androidx.annotation.NonNull;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.ShortBuffer;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioProcessor {
    /**
     * 预留一定长度，保证降噪后的音频至少有60秒
     */
    public static final int SIZE = 65 * AudioRecorder.SAMPLE_RATE;

    public static final int FRAME = 60 * AudioRecorder.SAMPLE_RATE;

    public static final int AUDIO_DATA = 0x800000;

    Handler handler = new Handler(Looper.getMainLooper(), AudioProcessor.this::handleMessage);

    ExecutorService service = Executors.newCachedThreadPool();

    AudioRecorder recorder = new AudioRecorder(service);

    volatile boolean isWorking = false;
    /**
     * 开始时间
     */
    long start;

    /**
     * 第几分钟
     */
    long minute;

    ShortBuffer writeBuffer;

    ShortBuffer readBuffer;

    public AudioProcessor() {
        recorder.setCallback(this::update);
        ByteBuffer buffer;

        buffer = ByteBuffer.allocateDirect(SIZE);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        writeBuffer = buffer.asShortBuffer();

        buffer = ByteBuffer.allocateDirect(SIZE);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        readBuffer = buffer.asShortBuffer();
    }

    public boolean start() {
        clear();
        isWorking = true;
        start = System.currentTimeMillis();
        minute = 0;
        if (!recorder.start()) {
            isWorking = false;
            return false;
        }
        return true;
    }

    public void stop() {
        isWorking = false;
        recorder.stop();
    }

    public boolean handleMessage(@NonNull Message msg) {
        if (msg.what == AUDIO_DATA) {
            ShortBuffer buffer = (ShortBuffer) msg.obj;
            long minute = msg.arg1 + (long) msg.arg2 << 32;
            return true;
        }
        return false;
    }

    void write(short[] buffer, int length) {
        int offset = 0;
        int copy = Math.min(writeBuffer.remaining(), length);

        if (copy > 0) {
            writeBuffer.put(buffer, offset, copy);
            offset += copy;
            length -= copy;
        }

        if (!writeBuffer.hasRemaining()) {
            swapBuffer();
            Message msg = handler.obtainMessage(AUDIO_DATA, readBuffer);
            msg.arg1 = (int) minute;
            msg.arg2 = (int) (minute >> (32));
            minute += 1;
            handler.sendMessage(msg);
        }

        if (length > 0) {
            writeBuffer.put(buffer, offset, length);
        }
    }

    /**
     * @param buffer 音频数据
     * @param length 数据长度，必须小于60 * 44100 * 2
     */
    public void update(short[] buffer, int length) {
        if (isWorking) {
            write(buffer, length);
        }
    }

    /**
     * 交换缓冲区
     */
    void swapBuffer() {
        //60秒末尾复制到读缓存
        readBuffer.clear();
        writeBuffer.position(FRAME);
        readBuffer.put(writeBuffer);
        writeBuffer.rewind();
        //读写缓存交换
        ShortBuffer temp = writeBuffer;
        writeBuffer = readBuffer;
        readBuffer = temp;
    }

    void clear() {
        writeBuffer.clear();
        readBuffer.clear();
    }

    static class Task implements Callable<Integer> {

        public Task() {

        }

        @Override
        public Integer call() throws Exception {
            return null;
        }
    }
}
