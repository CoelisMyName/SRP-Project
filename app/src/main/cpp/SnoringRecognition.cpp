#include <cassert>
#include <snore.h>
#include "log.h"
#include "utils.h"
#include "config.h"
#include "global.h"
#include "SPLBuilder.h"
#include "SnoreJNICallback.h"
#include "SnoringRecognition.h"

static const char *const TAG = "SnoringRecognition";

SnoringRecognition::SnoringRecognition(JNIEnv *env, jobject obj) : m_running(false),
                                                                   m_threadFinished(true),
                                                                   m_audio(this),
                                                                   m_callback(env, obj),
                                                                   m_builder(env),
                                                                   m_buffer(2, BUFFER_SIZE,
                                                                            BUFFER_SIZE -
                                                                            INPUT_SIZE) {
    m_frame = new double[FRAME_SIZE];
    m_thread = nullptr;
    m_start_time = 0;
    m_sample_rate = SAMPLE_RATE;
    snoreInitial();
}

SnoringRecognition::~SnoringRecognition() {
    shutdown();
    delete m_frame;
    snoreDestroy();
}

bool SnoringRecognition::start() {
    if (m_running) {
        LOG_D(TAG, "start(): is running");
        return true;
    }
    if (!m_threadFinished) {
        LOG_D(TAG, "start(): thread is not finished");
        return false;
    }
    m_buffer.clear();
    if (!m_audio.start()) {
        LOG_D(TAG, "start(): cannot start audio stream");
        return false;
    }
    m_running = true;
    m_threadFinished = false;
    m_start_time = currentTimeMillis();
    m_thread = new thread([this] {
        EnvHelper helper;
        JNIEnv *env = helper.getEnv();
        this->_run(env);
    });
    LOG_D(TAG, "start(): success");
    return true;
}

bool SnoringRecognition::stop() {
    if (!m_running) {
        LOG_D(TAG, "stop(): has already stopped");
        return true;
    }
    if (!m_audio.stop()) {
        LOG_D(TAG, "stop(): cannot stop audio stream");
        return false;
    }
    m_running = false;
    notify();
    m_thread->detach();
    delete m_thread;
    m_thread = nullptr;
    LOG_D(TAG, "stop(): success");
    return true;
}

uint64_t SnoringRecognition::getStartTime() const {
    return m_start_time;
}

double SnoringRecognition::getSampleRate() const {
    return m_sample_rate;
}

bool SnoringRecognition::isRunning() const {
    return m_running;
}

void SnoringRecognition::_run(JNIEnv *env) {
    m_threadFinished = false;
    char s_prof[2048];
    strcpy(s_prof, g_cache);
    strcat(s_prof, "/noiseprof.txt");
    uint64_t frames = 0;
    while (true) {
        wait();
        if (!m_running) break;
        while (m_buffer.ready() && m_running) {
            int16_t *data;
            uint32_t length;
            data = m_buffer.next(length);
            assert(data != nullptr);
            I16pcm src = {data, length, 1, m_sample_rate};
            I16pcm dst = {(int16_t *) malloc(sizeof(int16_t) * length), length, 1, m_sample_rate};
            F64pcm fds = {(double *) malloc(sizeof(double) * INPUT_SIZE), INPUT_SIZE, 1,
                          m_sample_rate};
            if (frames == 0) {
                LOG_D(TAG, "_run(): generate initial noise profile");
                generateNoiseProfile(src, 0.0, 1.0, s_prof);
            }
            reduceNoise(src, dst, s_prof, 0.21);
            for (uint32_t i = 0; i < INPUT_SIZE; ++i) {
                fds.raw[i] = dst.raw[i] / 32768.0;
            }
            ModelResult result;
            calculateModelResult(fds, result);
            for (uint32_t i = 0; i < result.s_size; ++i) {
                uint64_t sms = (result.starts[i] * 1000L) / 44100L;
                uint64_t ems = (result.ends[i] * 1000L) / 44100L;
                LOG_D(TAG, "_run(): frames: %llu, start: %llums, end: %llums, result: %s", frames,
                      sms, ems, result.label[i] > 0.5 ? "positive" : "negative");
                m_callback.onRecognize(env, frames, sms, ems, result.label[i] > 0.5);
            }
            if (result.n_start >= 0 && result.n_length >= 0) {
                LOG_D(TAG, "_run(): update noise profile");
                generateNoiseProfile(src, result.n_start, result.n_length, s_prof);
            }
            free(dst.raw);
            free(fds.raw);
            frames += 1;
        }
        if (!m_running) break;
    }
    m_threadFinished = true;
}

void SnoringRecognition::notify() {
//    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.notify_all();
}

void SnoringRecognition::wait() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait(lock);
}

void SnoringRecognition::shutdown() {
    m_audio.stop();
    m_running = false;
    notify();
    if (m_thread != nullptr) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
}

void SnoringRecognition::put(JNIEnv *env, int16_t *data, uint32_t length) {
    uint32_t wt = m_buffer.write(data, length);
    if (m_buffer.ready()) notify();
    uint32_t len = min(length, (uint32_t) FRAME_SIZE);
    for (uint32_t i = 0; i < len; ++i) {
        m_frame[i] = data[i] / 32768.0;
    }
    F64pcm dst = {m_frame, len, 1, m_sample_rate};
    SPL spl;
    calculateSPL(dst, spl);
    jlong timestamp = currentTimeMillis();
    jobject j_spl = m_builder.getNewSPL(env, timestamp, spl.a_sum, spl.c_sum, spl.z_sum, spl.freq,
                                        spl.a_pow, spl.c_pow, spl.z_pow);
    m_callback.onSPLDetect(env, j_spl);
    env->DeleteLocalRef(j_spl);
}