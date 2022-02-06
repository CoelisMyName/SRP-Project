#ifndef SRP_PROJECT_DEFAULTRENDER_H
#define SRP_PROJECT_DEFAULTRENDER_H

#include <GLES3/gl32.h>
#include "GLRender.h"
#include "AudioDataCallback.h"

class DefaultRender : public GLRender, public AudioDataCallback {

public:
    virtual ~DefaultRender() override = default;

    virtual void onAudioCallbackAttach() override {

    }

    virtual void onAudioDataStart(int64_t timestamp) override {

    }

    virtual void onAudioDataStop(int64_t timestamp) override {

    }

    virtual void onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) override {

    }

    virtual void onAudioCallbackDetach() override {

    }

    virtual void onRenderAttach() override {

    }

    virtual void onSurfaceCreate(int32_t width, int32_t height) override {

    }

    virtual void onSurfaceDraw() override {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    virtual void onSurfaceSizeChange(int32_t width, int32_t height) override {

    }

    virtual void onSurfaceDestroy() override {

    }

    virtual void onRenderDetach() override {

    }
};

#endif