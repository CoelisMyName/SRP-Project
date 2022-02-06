#ifndef SRP_PROJECT_AUDIORECORD_H
#define SRP_PROJECT_AUDIORECORD_H

#include <cstdint>
#include <oboe/Oboe.h>
#include "AudioSource.h"
#include "AudioDataDispatcher.h"

using std::atomic;
using std::shared_ptr;
using oboe::Result;
using oboe::Direction;
using oboe::AudioStream;
using oboe::AudioFormat;
using oboe::SharingMode;
using oboe::ChannelCount;
using oboe::AudioStreamBuilder;
using oboe::DataCallbackResult;
using oboe::AudioStreamCallback;
using oboe::SampleRateConversionQuality;

/**
 * 录音类，提供音频输入源，将采样数据通过 AudioDataDispatcher 进行分发
 */
class AudioRecord : public AudioSource, public AudioStreamCallback {
public:
    AudioRecord(AudioDataDispatcher *dispatcher);

    AudioRecord(AudioDataDispatcher *dispatcher, int32_t sample_rate /* 采样率 */,
                int32_t frame_size /* 每次采样的数据量 */);

    virtual ~AudioRecord();

    virtual int32_t getSampleRate() override;

    virtual int32_t getFrameSize() override;

    virtual int32_t getChannelCount() override;

    /**
     * 开始录音
     * @return true 为启动成功或已在运行，false 为启动失败
     */
    virtual bool start() override;

    /**
     * 停止录音
     * @return true 为停止成功或已停止，false 为停止出错
     */
    virtual bool stop() override;

    /**
     * 检查音频流是否还在运行
     * @return
     */
    virtual bool isRunning() override;

    /**
     * 音频流回调函数，当数据就绪时调用，函数属于 Oboe 模块
     * @param audioStream
     * @param audioData
     * @param numFrames
     * @return
     */
    virtual DataCallbackResult
    onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) override;

    /**
     * 音频流回调函数，当出现错误时调用，函数属于 Oboe 模块
     * @param stream
     * @param result
     * @return
     */
    virtual bool onError(AudioStream *stream, Result result) override;

private:
    atomic<bool> m_running;
    shared_ptr<AudioStream> m_stream{};
    int32_t m_sample_rate;
    int32_t m_frame_size;
    AudioDataDispatcher *m_dispatcher;

    /**
     * 开启录音流
     * @return
     */
    bool openStream();
};

#endif