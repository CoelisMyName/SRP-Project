#include <cstdint>
#include "log.h"
#include "utils.h"
#include "config.h"
#include "AudioRecord.h"

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

TAG(AudioRecord)

AudioRecord::AudioRecord(AudioDataDispatcher *dispatcher) : mRunning(false) {
    mSampleRate = SAMPLE_RATE;
    mFrameSize = FRAME_SIZE;
    mDispatcher = dispatcher;
    log_i("%s(): sample rate = %d, frame size = %d", __FUNCTION__, mSampleRate, mFrameSize);
}

AudioRecord::AudioRecord(AudioDataDispatcher *dispatcher, int32_t sample_rate, int32_t frame_size)
        : mRunning(false) {
    mSampleRate = sample_rate;
    mFrameSize = frame_size;
    mDispatcher = dispatcher;
    log_i("%s(): sample rate = %d, frame size = %d", __FUNCTION__, sample_rate, frame_size);
}

AudioRecord::~AudioRecord() {
    AudioRecord::stop();
}

int32_t AudioRecord::getSampleRate() {
    return mSampleRate;
}

int32_t AudioRecord::getFrameSize() {
    return mFrameSize;
}

int32_t AudioRecord::getChannelCount() {
    return 1;
}

bool AudioRecord::start() {
    if (mRunning) {
        log_i("%s(): %s", __FUNCTION__, "is already running");
        return true;
    }
    if (!openStream()) {
        log_d("%s(): %s", __FUNCTION__, "cannot open input stream");
        return false;
    }
    Result result = mStream->requestStart();
    if (result != Result::OK) {
        log_d("%s(): %s", __FUNCTION__, "call mStream->requestStart() failed");
        mStream->close();
        mStream.reset();
        return false;
    }
    if (mDispatcher != nullptr) {
        mDispatcher->dispatchStart(currentTimeMillis());
        log_i("%s(): %s", __FUNCTION__, "dispatch start message");
    }
    mRunning = true;
    log_i("%s(): %s", __FUNCTION__, "success");
    return true;
}

bool AudioRecord::stop() {
    if (!mRunning) {
        log_i("%s(): %s", __FUNCTION__, "has already stopped");
        return true;
    }
    mStream->stop();
    mStream->close();
    mStream.reset();
    mRunning = false;
    if (mDispatcher != nullptr) {
        mDispatcher->dispatchStop(currentTimeMillis());
    }
    log_i("%s(): %s", __FUNCTION__, "stream stopped");
    return true;
}

bool AudioRecord::isRunning() {
    return mRunning;
}

DataCallbackResult
AudioRecord::onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) {
    if (mDispatcher != nullptr) {
        auto data = (int16_t *) audioData;
//        int64_t timestamp = audioStream->getTimestamp(CLOCK_MONOTONIC).value().timestamp / 1000000;
        int64_t timestamp =
                currentTimeMillis() - (int64_t) ((1000.0 / getSampleRate()) * numFrames);
        mDispatcher->dispatchAudioData(timestamp, data, numFrames);
//        log_i("%s(): %s", __FUNCTION__, "dispatch data");
    }
    return DataCallbackResult::Continue;
}

bool AudioRecord::onError(AudioStream *stream, Result result) {
    log_e("%s(): error code = %d", __FUNCTION__, result);
    mDispatcher->dispatchStop(currentTimeMillis());
    log_d("%s(): %s", __FUNCTION__, "dispatch stop message");
    //TODO handle error
    return AudioStreamErrorCallback::onError(stream, result);
}

bool AudioRecord::openStream() {
    AudioStreamBuilder builder;
    builder.setFormat(AudioFormat::I16);
    builder.setDirection(Direction::Input);
    builder.setFormatConversionAllowed(true);
    builder.setSharingMode(SharingMode::Exclusive);
    builder.setSampleRate(mSampleRate);
    builder.setFramesPerDataCallback(mFrameSize);
    builder.setChannelCount(ChannelCount::Mono);
    builder.setSampleRateConversionQuality(SampleRateConversionQuality::Medium);
    builder.setDataCallback(this);
    builder.setErrorCallback(this);
    Result result = builder.openStream(mStream);
    log_i("%s(): %s", __FUNCTION__, "try to open stream");
    return result == Result::OK;
}