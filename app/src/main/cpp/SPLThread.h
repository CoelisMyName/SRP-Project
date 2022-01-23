#ifndef SRP_PROJECT_SPLTHREAD_H
#define SRP_PROJECT_SPLTHREAD_H

#include <jni.h>
#include <queue>
#include <thread>
#include <snore.h>
#include "BufferPool.h"
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
    /**
     * 应当传递全局引用类型的 ModuleController 对象
     * @param global_obj
     */
    SPLThread(jobject global_obj);

    virtual ~SPLThread() override;

    virtual void onAttach() override;

    virtual void onStart(int64_t timestamp) override;

    virtual void onStop(int64_t timestamp) override;

    virtual void onReceive(int64_t timestamp, int16_t *data, int32_t length) override;

    virtual void onDetach() override;

    void run(JNIEnv *env);

    void waitForExit();

private:
    thread m_thread;
    mutex m_mutex;
    condition_variable m_cond;
    BufferPool<int16_t> m_buffer_pool;
    queue<int64_t> m_timestamp;
    int32_t m_sample_rate;
    uint32_t m_size;

    volatile jobject m_obj;
    volatile DispatchState m_state;
    volatile int64_t m_start, m_stop;
    volatile int64_t m_frame;
    volatile int64_t m_sample_count;
    volatile bool m_exit;
    volatile bool m_alive;
};

#endif