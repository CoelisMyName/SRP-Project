#ifndef SRP_PROJECT_AUDIOSOURCE_H
#define SRP_PROJECT_AUDIOSOURCE_H

#include <cstdint>

/**
 * 音频源虚类，提供音频接口
 */
class AudioSource {
public:
    virtual ~AudioSource() = default;

    /**
     * 返回音频的采样率
     * @return
     */
    virtual int32_t getSampleRate() = 0;

    /**
     * 返回音频帧大小
     * @return
     */
    virtual int32_t getFrameSize() = 0;

    /**
     * 返回音频通道数
     * @return
     */
    virtual int32_t getChannelCount() = 0;

    /**
     * 启动音频
     * @return
     */
    virtual bool start() = 0;

    /**
     * 结束音频
     * @return
     */
    virtual bool stop() = 0;

    /**
     * 是否在运行
     * @return
     */
    virtual bool isRunning() = 0;
};

#endif
