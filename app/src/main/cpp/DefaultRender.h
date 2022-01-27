#ifndef SRP_PROJECT_DEFAULTRENDER_H
#define SRP_PROJECT_DEFAULTRENDER_H

#include <GLES3/gl32.h>
#include "GLRender.h"
#include "AudioDataCallback.h"

class DefaultRender : public GLRender, public AudioDataCallback {

public:
    virtual ~DefaultRender() = default;

    virtual void onAttach() {

    }

    virtual void onStart(int64_t timestamp) {

    }

    virtual void onStop(int64_t timestamp) {

    }

    virtual void onReceive(int64_t timestamp, int16_t *data, int32_t length) {

    }

    virtual void onDetach() {

    }

    virtual void onCreate(int32_t width, int32_t height) {

    }

    virtual void onDraw() {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    virtual void onChange(int32_t width, int32_t height) {

    }

    virtual void onDestroy() {

    }
};

#endif