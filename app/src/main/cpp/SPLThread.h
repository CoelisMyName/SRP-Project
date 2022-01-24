#ifndef SRP_PROJECT_SPLTHREAD_H
#define SRP_PROJECT_SPLTHREAD_H

#include <jni.h>
#include <queue>
#include <thread>
#include <snore.h>
#include "BufferPool.h"
#include "SPLJNICallback.h"
#include "AudioDataCallback.h"
#include "AudioDataDispatcher.h"

using std::mutex;
using std::queue;
using std::thread;
using std::unique_lock;
using std::condition_variable;
using snore::F64pcm;
using snore::snoreInitial;
using snore::snoreDestroy;
using snore::calculateSPL;

typedef snore::SPL LibSnoreSPL;

class SPLThread : public AudioDataCallback {
public:
    SPLThread(SPLJNICallback *callback);

    virtual ~SPLThread() override;

    virtual void onAttach() override;

    virtual void onStart(int64_t timestamp) override;

    virtual void onStop(int64_t timestamp) override;

    virtual void onReceive(int64_t timestamp, int16_t *data, int32_t length) override;

    virtual void onDetach() override;

    void run(JNIEnv *env);

    void waitForExit();

private:
    uint32_t m_size;
    int32_t m_sample_rate;

    thread m_thread;
    mutex m_mutex;
    condition_variable m_cond;

    SPLJNICallback *m_callback;

    BufferPool<int16_t> m_buffer_pool;
    queue<int64_t> m_timestamp;

    volatile DispatchState m_state;
    volatile int64_t m_start, m_stop;
    volatile int64_t m_frame;
    volatile int64_t m_sample_count;
    volatile bool m_exit;
    volatile bool m_alive;
};

#endif