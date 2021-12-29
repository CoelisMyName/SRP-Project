#ifndef LOG_H
#define LOG_H

#include <android/log.h>

/**
 * log宏定义
 */
#define LOG_D(tag, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)
#define LOG_I(tag, ...) __android_log_print(ANDROID_LOG_INFO, tag, __VA_ARGS__)
#define LOG_W(tag, ...) __android_log_print(ANDROID_LOG_WARN, tag, __VA_ARGS__)
#define LOG_E(tag, ...) __android_log_print(ANDROID_LOG_ERROR, tag, __VA_ARGS__)
#define LOG_F(tag, ...) __android_log_print(ANDROID_LOG_FATAL, tag, __VA_ARGS__)

#endif
