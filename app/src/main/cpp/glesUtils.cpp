#include <cstring>
#include <android/asset_manager.h>
#include "log.h"
#include "global.h"
#include "glesUtils.h"

TAG(glesUtils)

bool createShader(GLenum shaderType, const char *str, GLuint &shader, GLenum &error) {
    error = GL_NO_ERROR;
    shader = glCreateShader(shaderType);
    if (shader == 0) {
        error = glGetError();
        return false;
    }
    glShaderSource(shader, 1, &str, nullptr);
    GLint compiled = GL_FALSE;
    glCompileShader(shader);
    error = glGetError();
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint infoLogLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0) {
            auto infoLog = new GLchar[infoLogLen + 1];
            memset(infoLog, 0, sizeof(GLchar) * (infoLogLen + 1));
            glGetShaderInfoLog(shader, infoLogLen, nullptr, infoLog);
            log_e("%s(): cannot compile %s with error message: %s", __FUNCTION__,
                  getShaderTypeString(shader), infoLog);
            delete[] infoLog;
        }
        glDeleteShader(shader);
        shader = 0;
        return false;
    }
    return true;
}

bool createProgram(const char *vertStr, const char *fragStr, GLuint &program, GLenum &error) {
    program = 0;
    GLuint vert = 0, frag = 0, pgr = 0;
    error = GL_NO_ERROR;
    GLint linked = GL_FALSE;
    if (!createShader(GL_VERTEX_SHADER, vertStr, vert, error)) {
        goto exit;
    }
    if (!createShader(GL_FRAGMENT_SHADER, fragStr, frag, error)) {
        goto exit;
    }
    pgr = glCreateProgram();
    if (pgr == 0) {
        error = glGetError();
        goto exit;
    }
    glAttachShader(pgr, vert);
    glAttachShader(pgr, frag);
    glLinkProgram(pgr);
    error = glGetError();
    glGetProgramiv(pgr, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        GLint infoLogLen = 0;
        glGetProgramiv(pgr, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0) {
            auto infoLog = new GLchar[infoLogLen + 1];
            memset(infoLog, 0, sizeof(GLchar) * (infoLogLen + 1));
            glGetProgramInfoLog(pgr, infoLogLen, nullptr, infoLog);
            log_e("%s(): cannot link program with error message: %s", __FUNCTION__, infoLog);
            delete[] infoLog;
        }
        goto exit;
    }
    program = pgr;
    glDeleteShader(vert);
    glDeleteShader(frag);
    return true;
    exit:
    glDeleteShader(vert);
    glDeleteShader(frag);
    glDeleteProgram(pgr);
    return false;
}

bool loadProgramFromAssets(const char *vertPath, const char *fragPath, GLuint &program) {
    program = 0;
    if (g_assets == nullptr) return false;
    AAsset *asset;
    off_t size;
    int rd;
    GLenum error = GL_NO_ERROR;
    bool ret = false;
    asset = AAssetManager_open(g_assets, vertPath, AASSET_MODE_STREAMING);
    size = AAsset_getLength(asset);
    char *vert = new char[size + 1];
    memset(vert, 0, size + 1);
    rd = AAsset_read(asset, vert, size);
    AAsset_close(asset);
    asset = AAssetManager_open(g_assets, fragPath, AASSET_MODE_STREAMING);
    size = AAsset_getLength(asset);
    char *frag = new char[size + 1];
    memset(frag, 0, size + 1);
    rd = AAsset_read(asset, frag, size);
    AAsset_close(asset);
    log_i("%s(): %s", __FUNCTION__, "load shader file");
    ret = createProgram(vert, frag, program, error);
    if (!ret) {
        log_e("%s(): create program error: %s", __FUNCTION__, getErrorMessage(error));
    }
    delete[] vert;
    delete[] frag;
    return ret;
}

const char *getErrorMessage(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_INDEX:
            return "GL_INVALID_INDEX";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        default:
            return "UNKNOWN GL ERROR";
    }
}

const char *getShaderTypeString(GLenum type) {
    switch (type) {
        case GL_COMPUTE_SHADER:
            return "GL_COMPUTE_SHADER";
        case GL_FRAGMENT_SHADER:
            return "GL_FRAGMENT_SHADER";
        case GL_GEOMETRY_SHADER:
            return "GL_GEOMETRY_SHADER";
        case GL_TESS_CONTROL_SHADER:
            return "GL_TESS_CONTROL_SHADER";
        case GL_TESS_EVALUATION_SHADER:
            return "GL_TESS_EVALUATION_SHADER";
        case GL_VERTEX_SHADER:
            return "GL_VERTEX_SHADER";
        default:
            return "UNKNOWN GL SHADER";
    }
}