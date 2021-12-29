#ifndef SRP_PROJECT_SNOREAUDIORECORD_H
#define SRP_PROJECT_SNOREAUDIORECORD_H

#include <oboe/Oboe.h>
#include "SnoreAudioCallback.h"

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

class SnoreAudioRecord : public AudioStreamCallback {
public:
    SnoreAudioRecord(SnoreAudioCallback *cb);

    ~SnoreAudioRecord();

    bool start();

    bool stop();

    bool isRunning();

    DataCallbackResult
    onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) override;

    bool onError(AudioStream *stream, Result result) override;

private:
    atomic<bool> m_running;
    shared_ptr<AudioStream> m_stream;
    SnoreAudioCallback *m_cb;

    bool openStream();
};

#endif