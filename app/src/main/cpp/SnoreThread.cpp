#include <cassert>
#include <snore.h>
#include "log.h"
#include "utils.h"
#include "global.h"
#include "SnoreThread.h"

using std::mutex;
using std::thread;
using std::unique_lock;
using std::condition_variable;
using snore::I16pcm;
using snore::F64pcm;
using snore::ModelResult;
using snore::reduceNoise;
using snore::calculateModelResult;
using snore::generateNoiseProfile;

TAG(SnoreThread)

SnoreThread::SnoreThread(SnoreJNICallback *callback) : mCallback(callback),
                                                       mSize(SNORE_BUFFER_SIZE),
                                                       mBuffer(SAMPLE_RATE, SNORE_BUFFER_SIZE,
                                                               SNORE_PADDING_SIZE),
                                                       mThread([this] {
                                                           EnvHelper helper;
                                                           this->run(helper.getEnv());
                                                       }) {
    mSampleRate = SAMPLE_RATE;
    unique_lock<mutex> lock(mMutex);
    while (!mAlive) {
        mCond.notify_all();
        mCond.wait(lock);
    }
}

SnoreThread::~SnoreThread() {
    waitForExit();
}

void SnoreThread::onAudioCallbackAttach() {
}

void SnoreThread::onAudioDataStart(int64_t timestamp) {
    unique_lock<mutex> lock(mMutex);
    if (mExit || !mAlive) {
        mCond.notify_all();
        return;
    }
    mState = DispatchState::START;
    mStart = timestamp;
    mSampleCount = 0;
    mFrame = 0;
    mBuffer.clear();
    mCond.notify_all();
}

void SnoreThread::onAudioDataStop(int64_t timestamp) {
    unique_lock<mutex> lock(mMutex);
    if (mExit || !mAlive) {
        mCond.notify_all();
        return;
    }
    mState = DispatchState::STOP;
    mStop = timestamp;
    mCond.notify_all();
}

void SnoreThread::onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) {
    unique_lock<mutex> lock(mMutex);
    if (!mAlive || mExit) {
        mCond.notify_all();
        return;
    }
    int32_t write = mBuffer.put(timestamp, data, length);
    mSampleCount += write;
    if (write != length) {
        log_w("%s(): discard %d samples", __FUNCTION__, length - write);
    }
    if (mBuffer.ready()) {
        mCond.notify_all();
    }
}

void SnoreThread::onAudioCallbackDetach() {
}

void SnoreThread::run(JNIEnv *env) {
    // 初始化
    unique_lock<mutex> lock(mMutex);
    mAlive = true;
    mCond.notify_all();
    lock.unlock();

    SnoreJNICallback *callback = mCallback;
    int64_t start = 0, stop = 0, timestamp = 0;
    DispatchState state = DispatchState::STOP;
    int32_t size = mSize;
    int32_t rd = 0;
    int32_t frame = 0;
    bool exit, ready = false, onStart = false, onStop = false;
    auto buf1 = new int16_t[size];
    auto buf2 = new int16_t[size];
    auto buf3 = new double[size];
    char s_prof[256];
    char s_temp[256];
    checkAndMkdir(g_external_base, g_audio);

    strcpy(s_prof, g_external_base);
    strcat(s_prof, g_cache);
    strcat(s_prof, "/noise.prof");
    strcpy(s_temp, g_external_base);
    strcat(s_temp, g_cache);
    strcat(s_temp, "/1min.wav");

    while (true) {
        // sync state
        lock.lock();
        // 未退出，缓存池未准备好，状态未改变
        while (!mExit && !mBuffer.ready() && state == mState) {
            mCond.notify_all();
            mCond.wait(lock);
        }
        exit = mExit;
        if (state != mState && mState == DispatchState::START) {
            start = mStart;
            onStart = true;
            frame = 0;
        }

        if (state != mState && mState == DispatchState::STOP) {
            stop = mStop;
            onStop = true;
        }

        if (mBuffer.ready()) {
            rd = mBuffer.next(buf1, size, timestamp);
            ready = true;
        }
        state = mState;
        mCond.notify_all();
        lock.unlock();
        // do work
        if (exit) {
            break;
        }

        if (onStart) {
            onStart = false;
            callback->onStart(env, start);
        }

        if (ready) {
            ready = false;
            // 计算工作
            I16pcm src = {buf1, (uint32_t) rd, 1, (double) mSampleRate};
            I16pcm dst = {buf2, (uint32_t) rd, 1, (double) mSampleRate};
            F64pcm fds = {buf3, SNORE_INPUT_SIZE, 1, (double) mSampleRate};
            if (frame == 0) {
                log_i("%s(): generate initial noise profile", __FUNCTION__);
                generateNoiseProfile(src, 0.0, 1.0, s_prof);
            }
            reduceNoise(src, dst, s_prof, 0.21);
            //save wav file before crash
            writeWav(s_temp, dst.raw, dst.length, 1, 44100);
            for (uint32_t i = 0; i < SNORE_INPUT_SIZE; ++i) {
                fds.raw[i] = dst.raw[i] / 32768.0;
            }
            ModelResult result;
            calculateModelResult(fds, result);
            for (int32_t i = 0; i < result.s_size; ++i) {
                int64_t sms = (result.starts[i] * 1000L) / mSampleRate;
                int64_t ems = (result.ends[i] * 1000L) / mSampleRate;
//                log_i("%s(): frame: %d, start: %lld ms, end: %lld ms, result: %s", __FUNCTION__,
//                      frame, sms, ems, result.label[i] > 0.5 ? "positive" : "negative");
                Snore snore = {timestamp + sms, ems - sms, result.label[i] > 0.5, start};
                callback->onRecognize(env, snore);
            }
            if (result.n_start >= 0 && result.n_length >= 0) {
                log_i("%s(): update noise profile", __FUNCTION__);
                generateNoiseProfile(src, result.n_start, result.n_length, s_prof);
            }
            frame += 1;
            //TODO save some file
        }

        if (onStop) {
            onStop = false;
            callback->onStop(env, stop);
        }
    }

    delete[] buf1;
    delete[] buf2;
    delete[] buf3;
    lock.lock();
    mAlive = false;
    mCond.notify_all();
    lock.unlock();
}

void SnoreThread::waitForExit() {
    unique_lock<mutex> lock(mMutex);
    if (!mAlive) return;
    mExit = true;
    while (mAlive) {
        mCond.notify_all();
        mCond.wait(lock);
    }
    lock.unlock();
    mThread.join();
}