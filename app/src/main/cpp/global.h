#ifndef SRP_PROJECT_GLOBAL_H
#define SRP_PROJECT_GLOBAL_H

#include <jni.h>

extern JavaVM *g_jvm; // jvm虚拟机

extern jint g_version; // jni版本

extern const char *const g_cache; // cache路径

extern const char *const g_audio; // 保存音频路径

#endif
