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
        m_width = width;
        m_height = height;
        if (!m_init) {
            bool ret;
            ret = loadProgramFromAssets("shader/default.vert", "shader/default.frag", m_pgr);
            if (ret) {
                m_init = true;
            }
            glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
            const static GLfloat vertices[] = {
                    -0.5f, -0.5f, 0.0f,
                    0.5f, -0.5f, 0.0f,
                    0.0f, 0.5f, 0.0f
            };
            m_vbo = 0, m_vao = 0;
            glGenVertexArrays(1, &m_vao);
            glBindVertexArray(m_vao);

            glGenBuffers(1, &m_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
            glEnableVertexAttribArray(0);
        }
    }

    virtual void onSurfaceDraw() override {
        glUseProgram(m_pgr);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    virtual void onSurfaceSizeChange(int32_t width, int32_t height) override {
        m_width = width;
        m_height = height;
    }

    virtual void onSurfaceDestroy() override {
        if (m_init) {
            glDeleteProgram(m_pgr);
            m_pgr = 0;
            m_init = false;
            glDeleteBuffers(1, &m_vbo);
            glDeleteVertexArrays(1, &m_vao);
            m_vbo = 0;
            m_vao = 0;
        }
    }

    virtual void onRenderDetach() override {

    }

private:
    int32_t m_width = 0, m_height = 0;
    bool m_init = false;
    GLuint m_pgr = 0;
    GLuint m_vbo = 0, m_vao = 0;
};

#endif