#ifndef SRP_PROJECT_SNORETHREAD_H
#define SRP_PROJECT_SNORETHREAD_H

#include <jni.h>
#include <queue>
#include <thread>
#include "config.h"
#include "BufferPool.h"
#include "SnoreJNICallback.h"
#include "AudioDataCallback.h"
#include "AudioDataDispatcher.h"

using std::mutex;
using std::queue;
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
    uint32_t m_size;
    int32_t m_sample_rate;

    thread m_thread;
    mutex m_mutex;
    condition_variable m_cond;

    SnoreJNICallback *m_callback;

    BufferPool<int16_t> m_buffer_pool;
    queue<int64_t> m_timestamp;

    volatile DispatchState m_state;
    volatile int64_t m_start;
    volatile int64_t m_stop;
    volatile int64_t m_frame;
    volatile int64_t m_sample_count;
    volatile bool m_exit;
    volatile bool m_alive;
};

#endif