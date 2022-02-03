#ifndef SRP_PROJECT_SNORETHREAD_H
#define SRP_PROJECT_SNORETHREAD_H

#include <jni.h>
#include <queue>
#include <thread>
#include "config.h"
#include "AudioDataBuffer.h"
#include "SnoreJNICallback.h"
#include "AudioDataCallback.h"
#include "AudioDataDispatcher.h"

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

class SnoreThread : public AudioDataCallback {
public:
    SnoreThread(SnoreJNICallback *callback);

    virtual ~SnoreThread() override;

    virtual void onAttach() override;

    virtual void onStart(int64_t timestamp) override;

    virtual void onStop(int64_t timestamp) override;

    virtual void onReceive(int64_t timestamp, int16_t *data, int32_t length) override;

    virtual void onDetach() override;

    /**
     * 线程运行方法，所有运行工作在这个函数内
     * @param env
     */
    void run(JNIEnv *env);

    /**
     * 关闭线程，阻塞，直到线程结束
     */
    void waitForExit();

private:
    volatile DispatchState m_state = DispatchState::STOP;
    volatile int64_t m_start = 0;
    volatile int64_t m_stop = 0;
    volatile int64_t m_frame = 0;
    volatile int64_t m_sample_count = 0;
    volatile bool m_exit = false;
    volatile bool m_alive = false;

    int32_t m_size;
    int32_t m_sample_rate;

    SnoreJNICallback *m_callback;

    AudioDataBuffer<int16_t> m_buffer;

    mutex m_mutex;
    condition_variable m_cond;
    thread m_thread;
};

#endif