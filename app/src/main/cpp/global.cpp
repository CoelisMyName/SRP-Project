#include "global.h"

JavaVM *g_jvm = nullptr;///jvm虚拟机

jint g_version = 0;///jni版本

SnoringRecognition *g_snoringRecognition = nullptr;///鼾声识别

const char *const g_cache = "/storage/emulated/0/Android/data/com.scut/cache";///cache路径

const char *const g_audio = "/storage/emulated/0/Android/data/com.scut/audio";///保存音频路径