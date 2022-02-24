#include <cstring>
#include <android/asset_manager.h>
#include "log.h"
#include "global.h"
#include "glesUtils.h"
#include "WaveRender.h"

TAG(WaveRender)

WaveRender::WaveRender() {
    mAudioBufferSize = WAVE_RENDER_INPUT_SIZE + FRAME_SIZE;
}

WaveRender::~WaveRender() {

}

void WaveRender::onAudioCallbackAttach() {

}

void WaveRender::onAudioDataStart(int64_t timestamp) {

}

void WaveRender::onAudioDataStop(int64_t timestamp) {

}

void WaveRender::onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) {

}

void WaveRender::onAudioCallbackDetach() {

}

void WaveRender::onRenderAttach() {

}

void WaveRender::onSurfaceCreate(int32_t width, int32_t height) {
    mWidth = width;
    mHeight = height;
    if (!mInit) {
        bool ret;
        ret = loadProgramFromAssets("shader/wave.vert", "shader/wave.frag", mPgr);
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

void WaveRender::onSurfaceDraw() {
    glUseProgram(mPgr);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(mVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void WaveRender::onSurfaceSizeChange(int32_t width, int32_t height) {
    mWidth = width;
    mHeight = height;
}

void WaveRender::onSurfaceDestroy() {
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

void WaveRender::onRenderDetach() {

}