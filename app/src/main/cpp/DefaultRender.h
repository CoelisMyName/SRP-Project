#ifndef SRP_PROJECT_DEFAULTRENDER_H
#define SRP_PROJECT_DEFAULTRENDER_H

#if __ANDROID_API__ >= 24

#include <GLES3/gl32.h>

#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#include <cstring>
#include <android/asset_manager.h>
#include "log.h"
#include "global.h"
#include "GLRender.h"
#include "glesUtils.h"
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
        mWidth = width;
        mHeight = height;
        if (!mInit) {
            bool ret;
            ret = loadProgramFromAssets("shader/default.vert", "shader/default.frag", mPgr);
            if (ret) {
                mInit = true;
            }
            glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
            const static GLfloat vertices[] = {
                    -0.5f, -0.5f, 0.0f,
                    0.5f, -0.5f, 0.0f,
                    0.0f, 0.5f, 0.0f
            };
            mVbo = 0, mVao = 0;
            glGenVertexArrays(1, &mVao);
            glBindVertexArray(mVao);

            glGenBuffers(1, &mVbo);
            glBindBuffer(GL_ARRAY_BUFFER, mVbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
            glEnableVertexAttribArray(0);
        }
    }

    virtual void onSurfaceDraw() override {
        glUseProgram(mPgr);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(mVao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    virtual void onSurfaceSizeChange(int32_t width, int32_t height) override {
        mWidth = width;
        mHeight = height;
    }

    virtual void onSurfaceDestroy() override {
        if (mInit) {
            glDeleteProgram(mPgr);
            mPgr = 0;
            mInit = false;
            glDeleteBuffers(1, &mVbo);
            glDeleteVertexArrays(1, &mVao);
            mVbo = 0;
            mVao = 0;
        }
    }

    virtual void onRenderDetach() override {

    }

private:
    int32_t mWidth = 0, mHeight = 0;
    bool mInit = false;
    GLuint mPgr = 0;
    GLuint mVbo = 0, mVao = 0;
};

#endif