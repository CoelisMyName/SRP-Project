#include "global.h"

const JavaVM *g_jvm = nullptr;

jint g_version = 0;

const char *const g_external_base = "/storage/emulated/0/Android/data/com.scut";

const char *const g_cache = "/storage/emulated/0/Android/data/com.scut/cache";

const char *const g_audio = "/storage/emulated/0/Android/data/com.scut/audio";

AAssetManager *g_assets = nullptr;