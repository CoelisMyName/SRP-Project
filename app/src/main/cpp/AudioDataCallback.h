#ifndef SRP_PROJECT_AUDIODATACALLBACK_H
#define SRP_PROJECT_AUDIODATACALLBACK_H

#include <cstdint>

/**
 * 供 AudioDataDispatcher 处理的回调类，须自行解决线程同步，所有方法不要做耗时操作
 */
class AudioDataCallback {
public:
    virtual ~AudioDataCallback() = default;

    /**
     * 当对象添加到 AudioDataDispatcher 时调用一次
     */
    virtual void onAudioCallbackAttach() = 0;

    /**
     * 当收到启动消息的时候调用
     * @param timestamp
     */
    virtual void onAudioDataStart(int64_t timestamp) = 0;

    /**
     * 当收到停止消息的时候调用
     * @param timestamp
     */
    virtual void onAudioDataStop(int64_t timestamp) = 0;

    /**
     * 当收到数据时调用
     * @param timestamp
     * @param data
     * @param length
     */
    virtual void onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) = 0;

    /**
     * 当对象从 AudioDataDispatcher 移除时调用
     */
    virtual void onAudioCallbackDetach() = 0;
};

#endif