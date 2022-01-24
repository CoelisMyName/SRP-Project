#include <cassert>
#include <snore.h>
#include "log.h"
#include "utils.h"
#include "global.h"
#include "SnoreThread.h"
#include "SnoreJNICallback.h"

static const char *const TAG = "SnoreThread";

SnoreThread::SnoreThread(jobject global_obj) : m_state(STOP), m_start(0), m_stop(0), m_frame(0),
                                               m_sample_count(0), m_obj(nullptr), m_exit(false),
                                               m_alive(false), m_buffer_pool(3, SNORE_BUFFER_SIZE,
                                                                             SNORE_BUFFER_SIZE -
                                                                             SNORE_INPUT_SIZE),
                                               m_size(SNORE_BUFFER_SIZE),
                                               m_thread([this] {
                                                   JNIEnv *env;
                                                   g_jvm->AttachCurrentThread(&env, nullptr);
                                                   this->run(env);
                                                   g_jvm->DetachCurrentThread();
                                               }) {
    m_sample_rate = SAMPLE_RATE;
    unique_lock<mutex> lock(m_mutex);
    m_obj = global_obj;
    while (m_obj != nullptr) {
        m_cond.notify_all();
        m_cond.wait(lock);
    }
}

SnoreThread::~SnoreThread() {
    waitForExit();
}

void SnoreThread::onAttach() {
}

void SnoreThread::onStart(int64_t timestamp) {
    unique_lock<mutex> lock(m_mutex);
    if (m_exit || !m_alive) {
        m_cond.notify_all();
        return;
    }
    m_state = START;
    m_start = timestamp;
    m_sample_count = 0;
    m_frame = 0;
    m_buffer_pool.clear();
    while (!m_timestamp.empty()) {
        m_timestamp.pop();
    }
    m_cond.notify_all();
}

void SnoreThread::onStop(int64_t timestamp) {
    unique_lock<mutex> lock(m_mutex);
    if (m_exit || !m_alive) {
        m_cond.notify_all();
        return;
    }
    m_state = STOP;
    m_stop = timestamp;
    m_cond.notify_all();
}

void SnoreThread::onReceive(int64_t timestamp, int16_t *data, int32_t length) {
    unique_lock<mutex> lock(m_mutex);
    if (!m_alive || m_exit) {
        m_cond.notify_all();
        return;
    }
    if (m_sample_count / SNORE_INPUT_SIZE == m_frame) {
        m_timestamp.push(timestamp);
        m_frame += 1;
    }
    int32_t write = m_buffer_pool.write(data, length);
    m_sample_count += write;
    if (write != length) {
        log_w("%s(): discard %d samples", __FUNCTION__, length - write);
    }
    if (m_buffer_pool.ready()) {
        m_cond.notify_all();
    }
}

void SnoreThread::onDetach() {
}

void SnoreThread::run(JNIEnv *env) {
    m_alive = true;
    // 初始化
    unique_lock<mutex> lock(m_mutex);
    while (m_obj == nullptr) {
        m_cond.notify_all();
        m_cond.wait(lock);
    }
    jobject obj = env->NewLocalRef(m_obj);
    m_obj = nullptr;
    m_cond.notify_all();
    lock.unlock();
//    env->FindClass("com/scut/utils/ModuleController");
//    env->FindClass("com/scut/utils/Snore");
    SnoreJNICallback jniCallback(env, obj);
    int64_t start = 0, stop = 0, timestamp = 0;
    DispatchState state = STOP;
    uint32_t size = m_size;
    int32_t frame = 0;
    bool exit = false, ready = false, onStart = false, onStop = false;
    auto buf1 = new int16_t[size];
    auto buf2 = new int16_t[size];
    auto buf3 = new double[size];
    char s_prof[2048];
    strcpy(s_prof, g_cache);
    strcat(s_prof, "/noiseprof.txt");

    while (true) {
        // sync state
        lock.lock();
        // 未退出，缓存池未准备好，状态未改变
        while (!m_exit && !m_buffer_pool.ready() && state == m_state) {
            m_cond.wait(lock);
        }
        exit = m_exit;
        if (state != m_state && m_state == START) {
            start = m_start;
            onStart = true;
            frame = 0;
        }

        if (state != m_state && m_state == STOP) {
            stop = m_stop;
            onStop = true;
        }

        if (m_buffer_pool.ready()) {
            timestamp = m_timestamp.front();
            m_timestamp.pop();
            int16_t *ptr = m_buffer_pool.next(size);
            assert(ptr != nullptr);
            memcpy(buf1, ptr, size * sizeof(int16_t));
            ready = true;
        }
        state = m_state;
        m_cond.notify_all();
        lock.unlock();
        // do work
        if (exit) {
            break;
        }

        if (onStart) {
            onStart = false;
            jniCallback.onStart(start);
        }

        if (ready) {
            ready = false;
            // 计算工作
            I16pcm src = {buf1, size, 1, (double) m_sample_rate};
            I16pcm dst = {buf2, size, 1, (double) m_sample_rate};
            F64pcm fds = {buf3, SNORE_INPUT_SIZE, 1, (double) m_sample_rate};
            if (frame == 0) {
                log_i("%s(): generate initial noise profile", __FUNCTION__);
                generateNoiseProfile(src, 0.0, 1.0, s_prof);
            }
            reduceNoise(src, dst, s_prof, 0.21);
            for (uint32_t i = 0; i < SNORE_INPUT_SIZE; ++i) {
                fds.raw[i] = dst.raw[i] / 32768.0;
            }
            ModelResult result;
            calculateModelResult(fds, result);
            for (int32_t i = 0; i < result.s_size; ++i) {
                int64_t sms = (result.starts[i] * 1000L) / 44100L;
                int64_t ems = (result.ends[i] * 1000L) / 44100L;
                log_i("%s(): frame: %d, start: %lld ms, end: %lld ms, result: %s", __FUNCTION__,
                      frame, sms, ems, result.label[i] > 0.5 ? "positive" : "negative");
                Snore snore = {timestamp + sms, ems - sms, result.label[i] > 0.5, start};
                jniCallback.onRecognize(snore);
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
            jniCallback.onStop(stop);
        }
    }

    delete[] buf1;
    delete[] buf2;
    delete[] buf3;
    env->DeleteLocalRef(obj);
    lock.lock();
    m_alive = false;
    m_cond.notify_all();
    lock.unlock();
}

void SnoreThread::waitForExit() {
    unique_lock<mutex> lock(m_mutex);
    m_exit = true;
    while (m_alive) {
        m_cond.notify_all();
        m_cond.wait(lock);
    }
    lock.unlock();
    m_thread.join();
}