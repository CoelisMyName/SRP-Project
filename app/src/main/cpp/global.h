#ifndef SRP_PROJECT_GLOBAL_H
#define SRP_PROJECT_GLOBAL_H

#include <jni.h>
#include "SnoringRecognition.h"

extern JavaVM *g_jvm;

extern jint g_version;

extern SnoringRecognition *g_snoringRecognition;

extern const char *const g_cache;

extern const char *const g_audio;

#endif
