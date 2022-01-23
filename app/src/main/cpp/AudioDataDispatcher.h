#ifndef SRP_PROJECT_AUDIODATADISPATCHER_H
#define SRP_PROJECT_AUDIODATADISPATCHER_H

#include <thread>
#include <vector>
#include <cstdint>
#include "AudioDataCallback.h"

using std::mutex;
using std::thread;
using std::vector;

enum DispatchState {
    START, STOP
};

/**
 * 音频消息分发器，将音频消息状态分发给注册回调
 */
class AudioDataDispatcher {
public:
    AudioDataDispatcher();

    virtual ~AudioDataDispatcher();

    /**
     * 音频开始分发的消息
     * @param timestamp
     */
    void dispatchStart(int64_t timestamp);

    /**
     * 音频结束分发的消息
     * @param timestamp
     */
    void dispatchStop(int64_t timestamp);

    /**
     * 分发音频数据
     * @param timestamp
     * @param data
     * @param length
     */
    void dispatchAudioData(int64_t timestamp, int16_t *data, int32_t length);

    /**
     * 注册回调
     * @param callback
     */
    void registerCallback(AudioDataCallback *callback);

    /**
     * 取消回调注册
     * @param callback
     */
    void unregisterCallback(AudioDataCallback *callback);

private:
    vector<AudioDataCallback *> m_callbacks;
    mutex m_mutex;
    // 共享变量
    volatile DispatchState m_state;
    volatile int64_t m_start;
    volatile int64_t m_timestamp;
    volatile int64_t m_stop;
};

#endif