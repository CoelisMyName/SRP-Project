#include <cstring>
#include <android/asset_manager.h>
#include "log.h"
#include "global.h"
#include "glesUtils.h"
#include "WaveRender.h"

TAG(WaveRender)

WaveRender::WaveRender() {

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
    m_width = width;
    m_height = height;
    if (!m_init) {
        bool ret;
        ret = loadProgramFromAssets("shader/wave.vert", "shader/wave.frag", m_pgr);
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

void WaveRender::onSurfaceDraw() {
    glUseProgram(m_pgr);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void WaveRender::onSurfaceSizeChange(int32_t width, int32_t height) {
    m_width = width;
    m_height = height;
}

void WaveRender::onSurfaceDestroy() {
    if (m_init) {
        glDeleteProgram(m_pgr);
        m_pgr = 0;
        m_init = false;
        glDeleteBuffers(1, &m_vbo);
        glDeleteVertexArrays(1, &m_vao);
    }
}

void WaveRender::onRenderDetach() {

}