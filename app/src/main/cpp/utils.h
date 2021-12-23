#ifndef SRP_PROJECT_UTILS_H
#define SRP_PROJECT_UTILS_H

#include <cstdint>

/**
 * 写入到文件
 * @param dst 文件路径
 * @param i16_pcm pcm数据
 * @param length 数据长度，即采样总数
 * @param channel 通道数
 * @param sample_rate 采样率
 */
extern void
writeWav(const char *dst, int16_t *i16_pcm, uint32_t length, uint32_t channel,
         uint32_t sample_rate);

extern bool checkAndMkdir(const char *path);

#endif
