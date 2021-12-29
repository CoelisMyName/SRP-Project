#ifndef SRP_PROJECT_SNORINGRECOGNITION_H
#define SRP_PROJECT_SNORINGRECOGNITION_H

#include <jni.h>
#include <cstdint>
#include <thread>
#include "snore.h"
#include "BufferPool.h"
#include "SPLBuilder.h"
#include "SnoreAudioRecord.h"
#include "SnoreJNICallback.h"
#include "SnoreAudioCallback.h"

using std::min;
using std::mutex;
using std::atomic;
using std::thread;
using std::condition_variable;
using snore::SPL;
using snore::I16pcm;
using snore::F64pcm;
using snore::ModelResult;
using snore::calculateSPL;
using snore::snoreInitial;
using snore::snoreDestroy;
using snore::calculateModelResult;
using snore::generateNoiseProfile;
using snore::reduceNoise;

class SnoringRecognition : public SnoreAudioCallback {
public:
    SnoringRecognition(JNIEnv *env, jobject obj);

    virtual ~SnoringRecognition();

    bool start();

    bool stop();

    uint64_t getStartTime() const;

    double getSampleRate() const;

    bool isRunning() const;

    void put(JNIEnv *env, int16_t *data, uint32_t length) override;

    void _run(JNIEnv *env);

private:
    atomic<bool> m_running;
    atomic<bool> m_threadFinished;
    mutex m_mutex;
    condition_variable m_cond;
    thread *m_thread;
    BufferPool<int16_t> m_buffer;
    SnoreAudioRecord m_audio;
    SPLBuilder m_builder;
    SnoreJNICallback m_callback;
    uint64_t m_start_time;
    double *m_frame;
    double m_sample_rate;

    void notify();

    void wait();

    void shutdown();
};


#endif
