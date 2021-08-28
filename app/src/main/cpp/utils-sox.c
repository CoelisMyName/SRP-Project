#include <sox.h>
#include <assert.h>
#include <stdlib.h>
#include "config.h"

void initialSox() {
    assert(sox_init() == SOX_SUCCESS);
    assert(sox_format_init() == SOX_SUCCESS);
}

void quitSox() {
    assert(sox_quit() == SOX_SUCCESS);
    sox_format_quit();
}

void freeEffects(sox_effect_t *effects[], int count) {
    for (int i = 0; i < count; ++i)
        free(effects[i]);
}

sox_signalinfo_t getDefaultSignalInfo(uint64 size) {
    sox_signalinfo_t info = DEFAULT_SIGNAL_INFO;
    info.length = size / ((info.channels * info.precision) / 8);
    return info;
}

sox_encodinginfo_t getDefaultEncodingInfo() {
    sox_encodinginfo_t info = DEFAULT_ENCODING_INFO;
    return info;
}

/**
 * 生成噪声特征文件
 */
void generateNoiseProfile(void *data /* 输入s16le单通道pcm数据 */,
                          size_t size /* 数据大小 */,
                          double start /* trim效果器起始位置（单位：秒） */,
                          double duration /* trim效果器长度（单位：秒） */,
                          const char *filename /* noiseprof输出文件路径 */) {
    initialSox();
    /** double转字符串 */
    char start_str[32], duration_str[32];
    sprintf(start_str, "%lf", start);
    sprintf(duration_str, "%lf", duration);

    /** 获取文件格式描述信息 */
    sox_signalinfo_t inSignal_info = getDefaultSignalInfo(size);
    sox_encodinginfo_t inEncoding_info = getDefaultEncodingInfo();

    /** 输入输出描述符 */
    sox_format_t *input_ft = sox_open_mem_read(data, size, &inSignal_info, &inEncoding_info,
                                               DEFAULT_FILETYPE);
    sox_format_t *output_ft = sox_open_write("", &input_ft->signal, &input_ft->encoding, "null",
                                             NULL, NULL);

    /** 创建效果器链 */
    sox_effects_chain_t *chain = sox_create_effects_chain(&input_ft->encoding, &input_ft->encoding);

    sox_effect_t *effects[4];
    char *args[8];
    /** 获取效果器并添加到链上 */
    effects[0] = sox_create_effect(sox_find_effect("input"));
    args[0] = (char *) input_ft;
    sox_effect_options(effects[0], 1, args);
    sox_add_effect(chain, effects[0], &input_ft->signal, &input_ft->signal);

    effects[1] = sox_create_effect(sox_find_effect("trim"));
    args[0] = start_str;
    args[1] = duration_str;
    sox_effect_options(effects[1], 2, args);
    sox_add_effect(chain, effects[1], &input_ft->signal, &input_ft->signal);

    effects[2] = sox_create_effect(sox_find_effect("noiseprof"));
    args[0] = filename;
    sox_effect_options(effects[2], 1, args);
    sox_add_effect(chain, effects[2], &input_ft->signal, &input_ft->signal);

    effects[3] = sox_create_effect(sox_find_effect("output"));
    args[0] = (char *) output_ft;
    sox_effect_options(effects[3], 1, args);
    sox_add_effect(chain, effects[3], &input_ft->signal, &output_ft->signal);

    /** 启动效果器 */
    sox_flow_effects(chain, NULL, NULL);

    /** 释放资源 */
    freeEffects(effects, 4);
    sox_delete_effects_chain(chain);
    sox_close(input_ft);
    sox_close(output_ft);

    quitSox();
}

size_t reduceNoise(void *srt /* 输入s16le单通道pcm数据 */,
                   size_t srt_sz /* 数据大小 */,
                   void *dest /* 输出内存地址 */,
                   size_t dest_sz /* 输出内存大小 */,
                   const char *filename /* 噪声文件名 */,
                   double coefficient /* 降噪系数 */) {
    initialSox();
    /** double转字符串 */
    char coefficient_str[32];
    sprintf(coefficient_str, "%lf", coefficient);

    /** 获取文件格式描述信息 */
    sox_signalinfo_t inSignal_info = getDefaultSignalInfo(srt_sz);
    sox_encodinginfo_t inEncoding_info = getDefaultEncodingInfo();

    sox_signalinfo_t outSignal_info = getDefaultSignalInfo(0);
    sox_encodinginfo_t outEncoding_info = getDefaultEncodingInfo();

    /** 输入输出描述符 */
    sox_format_t *input_ft = sox_open_mem_read(srt, srt_sz, &inSignal_info, &inEncoding_info,
                                               DEFAULT_FILETYPE);
    sox_format_t *output_ft = sox_open_mem_write(dest, dest_sz, &outSignal_info, &outEncoding_info,
                                                 DEFAULT_FILETYPE, NULL);

    /** 创建效果器链 */
    sox_effects_chain_t *chain = sox_create_effects_chain(&input_ft->encoding,
                                                          &output_ft->encoding);

    sox_effect_t *effects[3];
    char *args[4];
    /** 获取效果器并添加到链上 */
    effects[0] = sox_create_effect(sox_find_effect("input"));
    char *args0[1];
    args0[0] = (char *) input_ft;
    sox_effect_options(effects[0], 1, args0);
    sox_add_effect(chain, effects[0], &input_ft->signal, &input_ft->signal);

    effects[1] = sox_create_effect(sox_find_effect("noisered"));
    args[0] = filename;
    args[1] = coefficient_str;
    sox_effect_options(effects[1], 2, args);
    sox_add_effect(chain, effects[1], &input_ft->signal, &input_ft->signal);

    effects[2] = sox_create_effect(sox_find_effect("output"));
    args[0] = (char *) output_ft;
    sox_effect_options(effects[2], 1, args);
    sox_add_effect(chain, effects[2], &input_ft->signal, &output_ft->signal);

    /** 启动效果器 */
    sox_flow_effects(chain, NULL, NULL);

    /** 释放资源 */
    freeEffects(effects, 3);
    sox_delete_effects_chain(chain);
    size_t length = output_ft->olength * 2;
    sox_close(input_ft);
    sox_close(output_ft);
    return length;
}