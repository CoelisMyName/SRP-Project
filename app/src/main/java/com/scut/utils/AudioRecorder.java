package com.scut.utils;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;

public class AudioRecorder {
    public static final String TAG = "AudioRecorder";

    public static final int SOURCE = MediaRecorder.AudioSource.MIC;
    public static final int SAMPLE_RATE = 44100;
    public static final int CHANNEL = AudioFormat.CHANNEL_IN_MONO;
    public static final int FORMAT = AudioFormat.ENCODING_PCM_16BIT;
    public static final int MIN_BUFFER_SIZE = AudioRecord.getMinBufferSize(SAMPLE_RATE, CHANNEL, FORMAT) * 8;
    public static final int BIT_DEPTH = 16;
    public static final int BYTE_RATE = BIT_DEPTH * SAMPLE_RATE / 8;

    volatile boolean recording = false;


    /* 0.05 sec */
    short[] buffer = new short[2205];

    ExecutorService service;

    Callback callback = null;

    Task task;

    Future<Integer> future;

    public AudioRecorder(ExecutorService service) {
        this.service = service;
    }

    public boolean start() {
        synchronized (this) {
            if (!recording) {
                if (task != null) {
                    boolean working = task.isWorking();
                    if (working) {
                        return false;
                    } else {
                        task = null;
                    }
                }
                recording = true;
                AudioRecord record = new AudioRecord(SOURCE, SAMPLE_RATE, CHANNEL, FORMAT, MIN_BUFFER_SIZE);
                task = new Task(record, buffer, this, callback);
                future = service.submit(task);
                return true;
            }
            return true;
        }
    }


    public synchronized void stop() {
        synchronized (this) {
            if (recording) {
                recording = false;
            }
        }
    }

    public synchronized boolean isRecording() {
        synchronized (this) {
            return recording;
        }
    }

    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    public interface Callback {

        void update(short[] buffer, int length);

    }

    static class Task implements Callable<Integer> {

        AudioRecord record;

        AudioRecorder recorder;

        Callback callback;

        volatile boolean working = false;

        short[] buffer;

        public Task(AudioRecord record, short[] buffer, AudioRecorder recorder, Callback callback) {
            this.record = record;
            this.buffer = buffer;
            this.recorder = recorder;
            this.callback = callback;
        }

        public void startWorking() {
            synchronized (this) {
                working = true;
                record.startRecording();
            }
        }

        public void stopWorking() {
            synchronized (this) {
                working = false;
                record.stop();
                record.release();
            }
            record = null;
            recorder = null;
            callback = null;
            buffer = null;
        }

        public synchronized boolean isWorking() {
            synchronized (this) {
                return working;
            }
        }

        @Override
        public Integer call() {
            startWorking();
            int length;
            while (recorder.isRecording()) {
                length = record.read(buffer, 0, buffer.length);
                if (length < 0) {
                    stopWorking();
                    Log.d(TAG, "call: pull audio data error (code: " + length + ")");
                    return 1;
                }
                if (callback != null) {
                    callback.update(buffer, length);
                }
            }
            stopWorking();
            return 0;
        }
    }
}
