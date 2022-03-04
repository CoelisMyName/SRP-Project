#include "log.h"
#include "utils.h"
#include "config.h"
#include "AudioDataDispatcher.h"

using std::mutex;
using std::vector;
using std::unique_lock;

TAG(AudioDataDispatcher)

AudioDataDispatcher::AudioDataDispatcher() {
    mCallbacks.reserve(10);
    mState = DispatchState::STOP;
    mStart = 0;
    mTimestamp = 0;
    mStop = 0;
    log_i("%s(): %s", __FUNCTION__, "initial");
}

AudioDataDispatcher::~AudioDataDispatcher() = default;

void AudioDataDispatcher::dispatchStart(int64_t timestamp) {
    unique_lock<mutex> lock(mMutex);
    mState = DispatchState::START;
    mStart = timestamp;
    for (auto cb : mCallbacks) {
        if (cb == nullptr) continue;
        cb->onAudioDataStart(timestamp);
    }
    log_i("%s(): timestamp = %lld", __FUNCTION__, timestamp);
}

void AudioDataDispatcher::dispatchStop(int64_t timestamp) {
    unique_lock<mutex> lock(mMutex);
    mState = DispatchState::STOP;
    mStop = timestamp;
    for (auto cb : mCallbacks) {
        if (cb == nullptr) continue;
        cb->onAudioDataStop(timestamp);
    }
    log_i("%s(): timestamp = %lld", __FUNCTION__, timestamp);
}

void AudioDataDispatcher::dispatchAudioData(int64_t timestamp, int16_t *data, int32_t length) {
#ifdef BENCHMARK
    int64_t sms, ems;
    sms = currentTimeMillis();
#endif
    unique_lock<mutex> lock(mMutex);
    mTimestamp = timestamp;
    for (auto cb : mCallbacks) {
        if (cb == nullptr) continue;
        cb->onAudioDataReceive(timestamp, data, length);
    }
//    log_i("%s(): timestamp = %lld, length = %d", __FUNCTION__, timestamp, length);
#ifdef BENCHMARK
    ems = currentTimeMillis();
    if (ems - sms > 40) {
        log_w("%s(): time cost = %lld", __FUNCTION__, ems - sms);
    }
#endif
}

void AudioDataDispatcher::registerCallback(AudioDataCallback *callback) {
    if (callback == nullptr) return;
    unique_lock<mutex> lock(mMutex);
    for (auto cb : mCallbacks) {
        if (cb == callback) return;
    }
    mCallbacks.push_back(callback);
    callback->onAudioCallbackAttach();
    if (mState == DispatchState::START) {
        callback->onAudioDataStart(mStart);
    }
    log_i("%s(): %s, %p", __FUNCTION__, "register callback", callback);
}

void AudioDataDispatcher::unregisterCallback(AudioDataCallback *callback) {
    if (callback == nullptr) return;
    unique_lock<mutex> lock(mMutex);
//    mCallbacks.erase(std::remove_if(mCallbacks.begin(), mCallbacks.end(), [callback](AudioDataCallback *cb){return cb == callback; }),
//                      mCallbacks.end());
    // 迭代器删除元素
    for (auto iter = mCallbacks.begin(); iter != mCallbacks.end();) {
        if (*iter == callback) {
            mCallbacks.erase(iter);
            callback->onAudioCallbackDetach();
        } else {
            iter += 1;
        }
    }
    log_i("%s(): %s, %p", __FUNCTION__, "unregister callback", callback);
}

void AudioDataDispatcher::clear() {
    unique_lock<mutex> lock(mMutex);
    for (AudioDataCallback *cb : mCallbacks) {
        if (cb == nullptr) continue;
        cb->onAudioCallbackDetach();
    }
    mCallbacks.clear();
}