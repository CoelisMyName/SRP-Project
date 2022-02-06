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

/**
 * 从 str 创建 shader
 * @param shaderType 输入：着色器类别
 * @param str 输入：着色器字符串
 * @param shader 输出：着色器
 * @param error 输出：错误代码
 * @return 输出：true 成功，false 失败
 */
extern bool createShader(GLenum shaderType, const char *str, GLuint &shader, GLenum &error);

/**
 * 从顶点着色器字符串和片元着色器字符串创建GL程序
 * @param vertStr 输入：顶点着色器
 * @param fragStr 输入：片元着色器
 * @param program 输出：程序
 * @param error 输出：错误代码
 * @return 输出：true 成功，false 失败
 */
extern bool createProgram(const char *vertStr, const char *fragStr, GLuint &program, GLenum &error);

/**
 * 从Assets加载GL程序
 * @param vertPath 输入：顶点着色器路径
 * @param fragPath 输入：片元着色器路径
 * @param program 输出：程序
 * @return 输出：true 成功，false 失败
 */
extern bool loadProgramFromAssets(const char *vertPath, const char *fragPath, GLuint &program);

/**
 * 获取错误代码字符串
 * @param error 错误代码
 * @return 错误字符串
 */
extern const char *getErrorMessage(GLenum error);

/**
 * 获取着色器类型字符串
 * @param type 着色器类型
 * @return 着色器字符串
 */
extern const char *getShaderTypeString(GLenum type);

#endif