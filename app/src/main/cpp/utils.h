#ifndef SRP_PROJECT_UTILS_H
#define SRP_PROJECT_UTILS_H

#include <jni.h>
#include <cstdint>

/**
 * JNIEnv帮助类，负责管理JNIEnv生命周期，使用这个则不建议使用AttachCurrentThread和DetachCurrentThread
 */
class EnvHelper {
public:
    EnvHelper();

    ~EnvHelper();

    JNIEnv *getEnv();

private:
    bool m_fg;
    JNIEnv *m_env;
};

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

/**
 * 检查目录是否存在，不存在则创建
 * @param path 目录路径
 * @return 存在为真，不存在则返回是否创建成功
 */
extern bool checkAndMkdir(const char *path);

/**
 * 获取毫秒时间戳
 * @return 毫秒时间戳
 */
extern int64_t currentTimeMillis();

#endif
