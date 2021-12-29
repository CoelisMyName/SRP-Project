#include <cstdio>
#include <dirent.h>
#include <cassert>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

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

bool checkAndMkdir(const char *path) {
    DIR *dir = opendir(path);
    if (dir != nullptr) {
        closedir(dir);
        return true;
    } else if (errno == ENOENT) {
        //TODO make it recursive
        int res = mkdir(path, 0777);
        return res == 0;
    } else {
        return false;
    }
}

uint64_t currentTimeMillis() {
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

EnvHelper::EnvHelper() {
    m_env = nullptr;
    jint res = g_jvm->GetEnv((void **) &m_env, g_version);
    if (res == JNI_OK) {
        m_fg = false;
    } else if (res == JNI_EDETACHED) {
        g_jvm->AttachCurrentThread(&m_env, nullptr);
        m_fg = true;
    } else {

    }
    assert(m_env != nullptr);
}

EnvHelper::~EnvHelper() {
    if (m_fg) {
        g_jvm->DetachCurrentThread();
    }
}

JNIEnv *EnvHelper::getEnv() {
    return m_env;
}
