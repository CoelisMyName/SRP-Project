#include "global.h"

JavaVM *g_jvm = nullptr;

jint g_version = 0;

const char *const g_external_base = "/storage/emulated/0/Android/data/com.scut";

const char *const g_cache = "cache";

const char *const g_audio = "audio";

AAssetManager *g_assets = nullptr;