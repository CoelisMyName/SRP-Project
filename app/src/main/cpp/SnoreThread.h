#ifndef SRP_PROJECT_SNORETHREAD_H
#define SRP_PROJECT_SNORETHREAD_H

#include <jni.h>
#include <queue>
#include <thread>
#include "config.h"
#include "PatientThread.h"
#include "AudioDataBuffer.h"
#include "SnoreJNICallback.h"
#include "AudioDataCallback.h"
#include "AudioDataDispatcher.h"

class SnoreThread : public AudioDataCallback {
public:
    SnoreThread(SnoreJNICallback *callback, PatientThread *patientThread);

    virtual ~SnoreThread() override;

    virtual void onAudioCallbackAttach() override;

    virtual void onAudioDataStart(int64_t timestamp) override;

    virtual void onAudioDataStop(int64_t timestamp) override;

    virtual void onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) override;

    virtual void onAudioCallbackDetach() override;

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
    volatile DispatchState mState = DispatchState::STOP;
    volatile int64_t mStart = 0;
    volatile int64_t mStop = 0;
    volatile int64_t mFrame = 0;
    volatile int64_t mSampleCount = 0;
    volatile bool mExit = false;
    volatile bool mAlive = false;

    int32_t mSize;
    int32_t mSampleRate;

    SnoreJNICallback *mCallback;
    PatientThread *mPatientThread;

    AudioDataBuffer<int16_t> mBuffer;

    std::mutex mMutex;
    std::condition_variable mCond;
    std::thread mThread;
};

#endif