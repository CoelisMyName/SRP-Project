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
    bool mFg;
    JNIEnv *mEnv;
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
 * 获取毫秒时间戳
 * @return 毫秒时间戳
 */
extern int64_t currentTimeMillis();

/**
 * 打印时间戳
 * 起码需要大小为24的缓冲区
 * @param str 输出字符串
 * @param millis 毫秒数
 * @return 字符串长度
 */
extern int32_t sprintTimeMillis(char *str, int64_t millis);

extern bool isBigEndian();

extern bool isLittleEndian();

/**
 * 传入 base （基目录）和 path (相对目录)，检查并递归创建目录，例如
 * 传入 "/storage/emulated/0/Android/data/com.scut" 作为 base 参数
 * 传入 "/cache" 作为 path 参数
 * 首先会检查 base 是否存在，然后递归创建 "/storage/emulated/0/Android/data/com.scut/cache" 目录
 * 最后返回是否创建成功或存在
 * 这个函数非常有用，因为往往很多路径并没有权限读写，使用这个方法可将文件夹创建限定在指定目录
 * @param base 基目录
 * @param path 相对目录
 * @return 存在即为 true，不存在即为 false
 */
extern bool checkAndMkdir(const char *base, const char *path);

#endif
