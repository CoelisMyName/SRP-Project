#ifndef SRP_PROJECT_WAVERENDER_H
#define SRP_PROJECT_WAVERENDER_H

#if __ANDROID_API__ >= 24

#include <GLES3/gl32.h>

#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#include <thread>
#include "GLRender.h"
#include "AudioDataBuffer.h"
#include "AudioDataCallback.h"

class WaveRender : public GLRender, public AudioDataCallback {
public:
    WaveRender();

    virtual ~WaveRender() override;

    virtual void onAudioCallbackAttach() override;

    virtual void onAudioDataStart(int64_t timestamp) override;

    virtual void onAudioDataStop(int64_t timestamp) override;

    virtual void onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) override;

    virtual void onAudioCallbackDetach() override;

    virtual void onRenderAttach() override;

    virtual void onSurfaceCreate(int32_t width, int32_t height) override;

    virtual void onSurfaceDraw() override;

    virtual void onSurfaceSizeChange(int32_t width, int32_t height) override;

    virtual void onSurfaceDestroy() override;

    virtual void onRenderDetach() override;

private:
    int32_t mWidth = 0, mHeight = 0;
    bool mInit = false;
    GLuint mPgr = 0;
    GLuint mVbo = 0, mVao = 0;

    std::mutex mMutex;

    int32_t mAudioBufferCapacity;
    int32_t mAudioBufferSize = 0;
    int16_t *mAudioBuffer;

    Queue<int16_t> mMaxQueue;
    Queue<int16_t> mMinQueue;

    int32_t mBufferCapacity;
    int32_t mBufferSize = 0;
    int16_t *mMaxBuffer;
    int16_t *mMinBuffer;
};

#endif