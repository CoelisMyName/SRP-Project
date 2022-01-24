#include "log.h"
#include "utils.h"
#include "config.h"
#include "AudioDataDispatcher.h"

using std::unique_lock;

TAG(AudioDataDispatcher)

AudioDataDispatcher::AudioDataDispatcher() {
    m_callbacks.reserve(10);
    m_state = STOP;
    m_start = 0;
    m_timestamp = 0;
    m_stop = 0;
    log_i("%s(): %s", __FUNCTION__, "initial");
}

AudioDataDispatcher::~AudioDataDispatcher() = default;

void AudioDataDispatcher::dispatchStart(int64_t timestamp) {
    unique_lock<mutex> lock(m_mutex);
    m_state = START;
    m_start = timestamp;
    for (auto cb : m_callbacks) {
        cb->onStart(timestamp);
    }
    log_i("%s(): timestamp = %lld", __FUNCTION__, timestamp);
}

void AudioDataDispatcher::dispatchStop(int64_t timestamp) {
    unique_lock<mutex> lock(m_mutex);
    m_state = STOP;
    m_stop = timestamp;
    for (auto cb : m_callbacks) {
        cb->onStop(timestamp);
    }
    log_i("%s(): timestamp = %lld", __FUNCTION__, timestamp);
}

void AudioDataDispatcher::dispatchAudioData(int64_t timestamp, int16_t *data, int32_t length) {
#ifdef BENCHMARK
    int64_t sms, ems;
    sms = currentTimeMillis();
#endif
    unique_lock<mutex> lock(m_mutex);
    m_timestamp = timestamp;
    for (auto cb : m_callbacks) {
        cb->onReceive(timestamp, data, length);
    }
    log_i("%s(): timestamp = %lld, length = %d", __FUNCTION__, timestamp, length);
#ifdef BENCHMARK
    ems = currentTimeMillis();
    if (ems - sms > 40) {
        log_w("%s(): time cost = %lld", __FUNCTION__, ems - sms);
    }
#endif
}

void AudioDataDispatcher::registerCallback(AudioDataCallback *callback) {
    unique_lock<mutex> lock(m_mutex);
    for (auto cb : m_callbacks) {
        if (cb == callback) return;
    }
    m_callbacks.push_back(callback);
    callback->onAttach();
    if (m_state == START) {
        callback->onStart(m_start);
    }
    log_i("%s(): %s", __FUNCTION__, "register callback");
}

void AudioDataDispatcher::unregisterCallback(AudioDataCallback *callback) {
    unique_lock<mutex> lock(m_mutex);
    // 迭代器删除元素
    for (auto iter = m_callbacks.begin(); iter != m_callbacks.end();) {
        if (*iter == callback) {
            m_callbacks.erase(iter);
            callback->onDetach();
        } else {
            iter += 1;
        }
    }
    log_i("%s(): %s", __FUNCTION__, "unregister callback");
}

void AudioDataDispatcher::clear() {
    unique_lock<mutex> lock(m_mutex);
    for (AudioDataCallback *cb : m_callbacks) {
        cb->onDetach();
    }
    m_callbacks.clear();
}
