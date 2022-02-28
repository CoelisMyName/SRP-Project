#ifndef SRP_PROJECT_SPLTHREAD_H
#define SRP_PROJECT_SPLTHREAD_H

#include <jni.h>
#include <queue>
#include <thread>
#include <snore.h>
#include "SPLJNICallback.h"
#include "AudioDataBuffer.h"
#include "AudioDataCallback.h"
#include "AudioDataDispatcher.h"

class SPLThread : public AudioDataCallback {
public:
    SPLThread(SPLJNICallback *callback);

    virtual ~SPLThread() override;

    virtual void onAudioCallbackAttach() override;

    virtual void onAudioDataStart(int64_t timestamp) override;

    virtual void onAudioDataStop(int64_t timestamp) override;

    virtual void onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) override;

    virtual void onAudioCallbackDetach() override;

    void run(JNIEnv *env);

    void waitForExit();

private:
    volatile DispatchState mState = DispatchState::STOP;
    volatile int64_t mStart = 0, mStop = 0;
    volatile int64_t mFrame = 0;
    volatile int64_t mSampleCount = 0;
    volatile bool mExit = false;
    volatile bool mAlive = false;

    int32_t mSize;
    int32_t mSampleRate;

    SPLJNICallback *mCallback;

    AudioDataBuffer<int16_t> mBuffer;

    std::mutex mMutex;
    std::condition_variable mCond;
    std::thread mThread;
};

#endif