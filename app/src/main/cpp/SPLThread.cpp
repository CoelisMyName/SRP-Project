#include <cassert>
#include "log.h"
#include "utils.h"
#include "config.h"
#include "SPLThread.h"
#include "SPLJNICallback.h"

TAG(SPLThread)

SPLThread::SPLThread(jobject global_obj) : m_frame(0), m_size(SPL_INPUT_SIZE), m_obj(nullptr),
                                           m_state(STOP),
                                           m_start(0), m_stop(0), m_sample_count(0), m_exit(false),
                                           m_alive(false), m_buffer_pool(3, SPL_INPUT_SIZE),
                                           m_thread([this] {
                                               EnvHelper helper;
                                               this->run(helper.getEnv());
                                           }) {
    m_sample_rate = SAMPLE_RATE;
    unique_lock<mutex> lock(m_mutex);
    m_obj = global_obj;
    while (m_obj != nullptr) {
        m_cond.notify_all();
        m_cond.wait(lock);
    }
}

SPLThread::~SPLThread() {
    waitForExit();
}

void SPLThread::onAttach() {
}

void SPLThread::onStart(int64_t timestamp) {
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

void SPLThread::onStop(int64_t timestamp) {
    unique_lock<mutex> lock(m_mutex);
    if (m_exit || !m_alive) {
        m_cond.notify_all();
        return;
    }
    m_state = STOP;
    m_stop = timestamp;
    m_cond.notify_all();
}

void SPLThread::onReceive(int64_t timestamp, int16_t *data, int32_t length) {
    unique_lock<mutex> lock(m_mutex);
    if (m_exit || !m_alive) {
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

void SPLThread::onDetach() {
}

void SPLThread::run(JNIEnv *env) {
    // initial
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

    SPLJNICallback jniCallback(env, obj);
    DispatchState state = STOP;
    bool exit = false, ready = false, onStart = false, onStop = false;
    uint32_t size = m_size;
    int64_t start = 0, stop = 0, timestamp = 0;
    auto buf1 = new int16_t[m_size];
    auto buf2 = new double[m_size];

    while (true) {
        // sync
        lock.lock();
        while (!m_exit && !m_buffer_pool.ready() && state == m_state) {
            m_cond.wait(lock);
        }
        exit = m_exit;

        if (state != m_state && m_state == START) {
            start = m_start;
            onStart = true;
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
            for (uint32_t i = 0; i < size; ++i) {
                buf2[i] = buf1[i] / 32768.0;
            }
            F64pcm dst = {buf2, size, 1, (double) m_sample_rate};
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
            jniCallback.onDetect(spl);
        }

        if (onStop) {
            onStop = false;
            jniCallback.onStop(stop);
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
    m_exit = true;
    while (m_alive) {
        m_cond.notify_all();
        m_cond.wait(lock);
    }
    lock.unlock();
    m_thread.join();
}