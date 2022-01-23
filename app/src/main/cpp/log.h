#ifndef LOG_H
#define LOG_H

#include <android/log.h>
#include "config.h"

#define TAG(str) static const char *const TAG = #str;

#ifdef ENABLE_LOG

#ifdef ENABLE_LOG_I
#define log_i(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#else
#define log_i(...)
#endif

#ifdef ENABLE_LOG_D
#define log_d(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define log_d(...)
#endif

#ifdef ENABLE_LOG_W
#define log_w(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#else
#define log_w(...)
#endif

#ifdef ENABLE_LOG_E
#define log_e(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#else
#define log_e(...)
#endif

#ifdef ENABLE_LOG_F
#define log_f(...) __android_log_print(ANDROID_LOG_FATAL, TAG, __VA_ARGS__)
#else
#define log_f(...)
#endif

#else
#define log_i(...)
#define log_d(...)
#define log_w(...)
#define log_e(...)
#define log_f(...)
#endif

#endif