#include <cassert>
#include "log.h"
#include "utils.h"
#include "config.h"
#include "SPLThread.h"
#include "SPLJNICallback.h"

using std::mutex;
using std::thread;
using std::unique_lock;
using std::condition_variable;
using snore::F64pcm;
using snore::calculateSPL;

TAG(SPLThread)

SPLThread::SPLThread(SPLJNICallback *callback) : mCallback(callback), mSize(SPL_INPUT_SIZE),
                                                 mBuffer(SAMPLE_RATE, SPL_INPUT_SIZE, 0),
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

SPLThread::~SPLThread() {
    waitForExit();
}

void SPLThread::onAudioCallbackAttach() {
}

void SPLThread::onAudioDataStart(int64_t timestamp) {
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

void SPLThread::onAudioDataStop(int64_t timestamp) {
    unique_lock<mutex> lock(mMutex);
    if (mExit || !mAlive) {
        mCond.notify_all();
        return;
    }
    mState = DispatchState::STOP;
    mStop = timestamp;
    mCond.notify_all();
}

void SPLThread::onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) {
    unique_lock<mutex> lock(mMutex);
    if (mExit || !mAlive) {
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

void SPLThread::onAudioCallbackDetach() {
}

void SPLThread::run(JNIEnv *env) {
    // 初始化
    unique_lock<mutex> lock(mMutex);
    mAlive = true;
    mCond.notify_all();
    lock.unlock();

    SPLJNICallback *callback = mCallback;
    DispatchState state = DispatchState::STOP;
    bool exit, ready = false, onStart = false, onStop = false;
    int32_t size = mSize;
    int32_t rd = 0;
    int64_t start = 0, stop = 0, timestamp = 0;
    auto buf1 = new int16_t[size];
    auto buf2 = new double[size];

    while (true) {
        // sync
        lock.lock();
        while (!mExit && !mBuffer.ready() && state == mState) {
            mCond.notify_all();
            mCond.wait(lock);
        }
        exit = mExit;

        if (state != mState && mState == DispatchState::START) {
            start = mStart;
            onStart = true;
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
            for (uint32_t i = 0; i < size; ++i) {
                buf2[i] = buf1[i] / 32768.0;
            }
            F64pcm dst = {buf2, (uint32_t) rd, 1, (double) mSampleRate};
            LibSnoreSPL libSnoreSpl;
            calculateSPL(dst, libSnoreSpl);
            SPL spl;
            spl.timestamp = timestamp;
            spl.a_sum = libSnoreSpl.a_sum;
            spl.c_sum = libSnoreSpl.c_sum;
            spl.z_sum = libSnoreSpl.z_sum;
            for (int32_t i = 0; i < 8; ++i) {
                spl.a_pow[i] = libSnoreSpl.a_pow[i];
                spl.c_pow[i] = libSnoreSpl.c_pow[i];
                spl.z_pow[i] = libSnoreSpl.z_pow[i];
            }
//            log_i("%s(): spl detect", __FUNCTION__);
            callback->onDetect(env, spl);
        }

        if (onStop) {
            onStop = false;
            callback->onStop(env, stop);
        }
    }
    // exit
    delete[] buf1;
    delete[] buf2;
    lock.lock();
    mAlive = false;
    mCond.notify_all();
    lock.unlock();
}

void SPLThread::waitForExit() {
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