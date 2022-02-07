#ifndef SRP_PROJECT_WAVERENDER_H
#define SRP_PROJECT_WAVERENDER_H

#if __ANDROID_API__ >= 24

#include <GLES3/gl32.h>

#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#include "GLRender.h"
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

    void onRenderDetach() override;

private:
    int32_t m_width = 0, m_height = 0;
    bool m_init = false;
    GLuint m_pgr = 0;
    GLuint m_vbo = 0, m_vao = 0;
};

#endif