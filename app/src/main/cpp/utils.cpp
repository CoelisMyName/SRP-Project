#include <cstdio>
#include <dirent.h>
#include <cassert>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>

#ifdef __cplusplus
extern "C" {
#endif

#include "sox.h"

#ifdef __cplusplus
}
#endif

#include "utils.h"
#include "global.h"

void initSox() {
    assert(sox_init() == SOX_SUCCESS);
}

void quitSox() {
    assert(sox_quit() == SOX_SUCCESS);
}

void
writeWav(const char *dst, int16_t *i16_pcm, uint32_t length, uint32_t channel,
         uint32_t sample_rate) {
    initSox();
    sox_signalinfo_t sig_info_i = {(sox_rate_t) sample_rate, channel, 16, 0, nullptr};
    sox_encodinginfo_t enc_info_i = {SOX_ENCODING_SIGN2, 16, 1 / 0.0, sox_option_no, sox_option_no,
                                     sox_option_no, sox_false};
    sox_signalinfo_t sig_info_o = {(sox_rate_t) sample_rate, channel, 16, 0, nullptr};
    sox_encodinginfo_t enc_info_o = {SOX_ENCODING_SIGN2, 16, 1 / 0.0, sox_option_no, sox_option_no,
                                     sox_option_no, sox_false};

    sox_format_t *fmt_i = sox_open_mem_read(i16_pcm, length * 2, &sig_info_i, &enc_info_i, "raw");
    sox_format_t *fmt_o = sox_open_write(dst, &sig_info_o, &enc_info_o, "wav", nullptr, nullptr);

    sox_effects_chain_t *chain = sox_create_effects_chain(&fmt_i->encoding, &fmt_o->encoding);
    sox_effect_t *ef_i, *ef_o;

    ef_i = sox_create_effect(sox_find_effect("input"));
    ef_o = sox_create_effect(sox_find_effect("output"));

    char *arg_i[] = {(char *) fmt_i};
    sox_effect_options(ef_i, 1, arg_i);
    char *arg_o[] = {(char *) fmt_o};
    sox_effect_options(ef_o, 1, arg_o);
    sox_add_effect(chain, ef_i, &fmt_i->signal, &fmt_i->signal);
    sox_add_effect(chain, ef_o, &fmt_i->signal, &fmt_o->signal);

    sox_flow_effects(chain, nullptr, nullptr);

    free(ef_i);
    free(ef_o);
    sox_delete_effects_chain(chain);
    sox_close(fmt_i);
    sox_close(fmt_o);
    quitSox();
}

int64_t currentTimeMillis() {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int32_t sprintTimeMillis(char *str, int64_t millis) {
    time_t time = millis / 1000;
    tm result{};
    localtime_r(&time, &result);
    //2022-02-24 19:13:48.134
    int32_t size = strftime(str, 50, "%F %X", &result);
    size += sprintf(&str[size], ".%03lld", millis % 1000);
    return size;
}

EnvHelper::EnvHelper() {
    mEnv = nullptr;
    jint res = g_jvm->GetEnv((void **) &mEnv, g_version);
    if (res == JNI_OK) {
        mFg = false;
    } else if (res == JNI_EDETACHED) {
        g_jvm->AttachCurrentThread(&mEnv, nullptr);
        mFg = true;
    } else {

    }
    assert(mEnv != nullptr);
}

EnvHelper::~EnvHelper() {
    if (mFg) {
        g_jvm->DetachCurrentThread();
    }
}

JNIEnv *EnvHelper::getEnv() {
    return mEnv;
}

bool isBigEndian() {
    static int32_t val = 0x12345678;
    auto ptr = (int8_t *) &val;
    return *ptr == 0x12;
}

bool isLittleEndian() {
    static int32_t val = 0x12345678;
    auto ptr = (int8_t *) &val;
    return *ptr == 0x78;
}

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

bool checkDirExist(const char *path) {
    DIR *dir;
    if (access(path, F_OK) == 0) {
        dir = opendir(path);
        if (dir == nullptr) {
            return false;
        }
        closedir(dir);
        return true;
    }
    return false;
}

bool isAlphabet(char ch) {
    return ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z';
}

bool isWindowsDisk(const char *path) {
    int32_t len = strlen(path);
    return len == 3 && isAlphabet(path[0]) && path[1] == ':' && path[2] == PATH_SEPARATOR;
}

/**
 * 检查目录是否存在并创建目录
 * @param base
 * @param path
 * @return
 */
bool checkAndMkdir(const char *base, const char *path) {
    const int32_t len_base = strlen(base);
    const int32_t len_path = strlen(path);
#ifdef WIN32
    if(!checkDirExist(base) && !isWindowsDisk(base)) {
        return false;
    }
#else
    if (!checkDirExist(base)) {
        return false;
    }
#endif
    printf("base exist\n");
    char str[len_base + len_path + 2];
    strcpy(str, base);
    int32_t len_str = len_base;
    int32_t name_start = 0, name_end;
    while (path[name_start] != 0) {
        while (path[name_start] == PATH_SEPARATOR) name_start += 1;
        name_end = name_start;
        while (path[name_end] != PATH_SEPARATOR && path[name_end] != 0) name_end += 1;
        int32_t len_name = name_end - name_start;
        if (len_name == 0) continue;
        str[len_str] = PATH_SEPARATOR;
        len_str += 1;
        memcpy(&str[len_str], &path[name_start], len_name);
        name_start = name_end;
        len_str += len_name;
        str[len_str] = 0;
        printf("now check %s\n", str);
        if (!checkDirExist(str)) {
#ifdef WIN32
            int res = mkdir(str);
#else
            int res = mkdir(str, 0777);
#endif
            if (res != 0) return false;
        }
    }
    return true;
}