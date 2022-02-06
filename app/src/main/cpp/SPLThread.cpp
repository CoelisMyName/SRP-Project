#include <cassert>
#include "log.h"
#include "utils.h"
#include "config.h"
#include "SPLThread.h"
#include "SPLJNICallback.h"

TAG(SPLThread)

SPLThread::SPLThread(SPLJNICallback *callback) : m_callback(callback), m_size(SPL_INPUT_SIZE),
                                                 m_buffer(SAMPLE_RATE, SPL_INPUT_SIZE, 0),
                                                 m_thread([this] {
                                                     EnvHelper helper;
                                                     this->run(helper.getEnv());
                                                 }) {
    m_sample_rate = SAMPLE_RATE;
    unique_lock<mutex> lock(m_mutex);
    while (!m_alive) {
        m_cond.notify_all();
        m_cond.wait(lock);
    }
}

SPLThread::~SPLThread() {
    waitForExit();
}

void SPLThread::onAudioCallbackAttach() {
}

void SPLThread::onAudioDataStart(int64_t timestamp) {
    unique_lock<mutex> lock(m_mutex);
    if (m_exit || !m_alive) {
        m_cond.notify_all();
        return;
    }
    m_state = DispatchState::START;
    m_start = timestamp;
    m_sample_count = 0;
    m_frame = 0;
    m_buffer.clear();
    m_cond.notify_all();
}

void SPLThread::onAudioDataStop(int64_t timestamp) {
    unique_lock<mutex> lock(m_mutex);
    if (m_exit || !m_alive) {
        m_cond.notify_all();
        return;
    }
    m_state = DispatchState::STOP;
    m_stop = timestamp;
    m_cond.notify_all();
}

void SPLThread::onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) {
    unique_lock<mutex> lock(m_mutex);
    if (m_exit || !m_alive) {
        m_cond.notify_all();
        return;
    }
    int32_t write = m_buffer.put(timestamp, data, length);
    m_sample_count += write;
    if (write != length) {
        log_w("%s(): discard %d samples", __FUNCTION__, length - write);
    }
    if (m_buffer.ready()) {
        m_cond.notify_all();
    }
}

void SPLThread::onAudioCallbackDetach() {
}

void SPLThread::run(JNIEnv *env) {
    // 初始化
    unique_lock<mutex> lock(m_mutex);
    m_alive = true;
    m_cond.notify_all();
    lock.unlock();

    SPLJNICallback *callback = m_callback;
    DispatchState state = DispatchState::STOP;
    bool exit, ready = false, onStart = false, onStop = false;
    int32_t size = m_size;
    int32_t rd = 0;
    int64_t start = 0, stop = 0, timestamp = 0;
    auto buf1 = new int16_t[size];
    auto buf2 = new double[size];

    while (true) {
        // sync
        lock.lock();
        while (!m_exit && !m_buffer.ready() && state == m_state) {
            m_cond.notify_all();
            m_cond.wait(lock);
        }
        exit = m_exit;

        if (state != m_state && m_state == DispatchState::START) {
            start = m_start;
            onStart = true;
        }

        if (state != m_state && m_state == DispatchState::STOP) {
            stop = m_stop;
            onStop = true;
        }

        if (m_buffer.ready()) {
            rd = m_buffer.next(buf1, size, timestamp);
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
            callback->onStart(env, start);
        }

        if (ready) {
            ready = false;
            for (uint32_t i = 0; i < size; ++i) {
                buf2[i] = buf1[i] / 32768.0;
            }
            F64pcm dst = {buf2, (uint32_t) rd, 1, (double) m_sample_rate};
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
            log_i("%s(): spl detect", __FUNCTION__);
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
    m_alive = false;
    m_cond.notify_all();
    lock.unlock();
}

void SPLThread::waitForExit() {
    unique_lock<mutex> lock(m_mutex);
    if (!m_alive) return;
    m_exit = true;
    while (m_alive) {
        m_cond.notify_all();
        m_cond.wait(lock);
    }
    lock.unlock();
    m_thread.join();
}