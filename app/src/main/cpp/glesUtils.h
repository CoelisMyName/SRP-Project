#ifndef SRP_PROJECT_GLESUTILS_H
#define SRP_PROJECT_GLESUTILS_H

#if __ANDROID_API__ >= 24

#include <GLES3/gl32.h>

#elif __ANDROID_API__ >= 21

#include <GLES3/gl31.h>

#else

#include <GLES3/gl3.h>

#endif

//class GLProgram {
//public:
//    GLProgram();
//
//    virtual ~GLProgram();
//
//    bool attachShader(GLuint shader);
//
//    bool linkProgram();
//
//private:
//    GLuint m_program;
//};

extern bool createShader(GLenum shaderType, const char *str, GLuint &shader, GLenum &error);

extern bool createProgram(const char *vertStr, const char *fragStr, GLuint &program, GLenum &error);

extern bool loadProgramFromAssets(const char *vertPath, const char *fragPath, GLuint &program);

extern const char *getErrorMessage(GLenum error);

extern const char *getShaderTypeString(GLenum type);

#endif