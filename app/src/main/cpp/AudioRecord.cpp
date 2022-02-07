#include <cstdint>
#include "log.h"
#include "utils.h"
#include "config.h"
#include "AudioRecord.h"

TAG(AudioRecord)

AudioRecord::AudioRecord(AudioDataDispatcher *dispatcher) : m_running(false) {
    m_sample_rate = SAMPLE_RATE;
    m_frame_size = FRAME_SIZE;
    m_dispatcher = dispatcher;
    log_i("%s(): sample rate = %d, frame size = %d", __FUNCTION__, m_sample_rate, m_frame_size);
}

AudioRecord::AudioRecord(AudioDataDispatcher *dispatcher, int32_t sample_rate, int32_t frame_size)
        : m_running(false) {
    m_sample_rate = sample_rate;
    m_frame_size = frame_size;
    m_dispatcher = dispatcher;
    log_i("%s(): sample rate = %d, frame size = %d", __FUNCTION__, sample_rate, frame_size);
}

AudioRecord::~AudioRecord() {
    AudioRecord::stop();
}

int32_t AudioRecord::getSampleRate() {
    return m_sample_rate;
}

int32_t AudioRecord::getFrameSize() {
    return m_frame_size;
}

int32_t AudioRecord::getChannelCount() {
    return 1;
}

bool AudioRecord::start() {
    if (m_running) {
        log_i("%s(): %s", __FUNCTION__, "is already running");
        return true;
    }
    if (!openStream()) {
        log_d("%s(): %s", __FUNCTION__, "cannot open input stream");
        return false;
    }
    Result result = m_stream->requestStart();
    if (result != Result::OK) {
        log_d("%s(): %s", __FUNCTION__, "call m_stream->requestStart() failed");
        m_stream->close();
        m_stream.reset();
        return false;
    }
    if (m_dispatcher != nullptr) {
        m_dispatcher->dispatchStart(currentTimeMillis());
        log_i("%s(): %s", __FUNCTION__, "dispatch start message");
    }
    m_running = true;
    log_i("%s(): %s", __FUNCTION__, "success");
    return true;
}

bool AudioRecord::stop() {
    if (!m_running) {
        log_i("%s(): %s", __FUNCTION__, "has already stopped");
        return true;
    }
    m_stream->stop();
    m_stream->close();
    m_stream.reset();
    m_running = false;
    if (m_dispatcher != nullptr) {
        m_dispatcher->dispatchStop(currentTimeMillis());
    }
    log_i("%s(): %s", __FUNCTION__, "stream stopped");
    return true;
}

bool AudioRecord::isRunning() {
    return m_running;
}

DataCallbackResult
AudioRecord::onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) {
    if (m_dispatcher != nullptr) {
        auto data = (int16_t *) audioData;
//        int64_t timestamp = audioStream->getTimestamp(CLOCK_MONOTONIC).value().timestamp / 1000000;
        int64_t timestamp =
                currentTimeMillis() - (int64_t) ((1000.0 / getSampleRate()) * numFrames);
        m_dispatcher->dispatchAudioData(timestamp, data, numFrames);
        log_i("%s(): %s", __FUNCTION__, "dispatch data");
    }
    return DataCallbackResult::Continue;
}

bool AudioRecord::onError(AudioStream *stream, Result result) {
    log_e("%s(): error code = %d", __FUNCTION__, result);
    m_dispatcher->dispatchStop(currentTimeMillis());
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
    builder.setSampleRate(m_sample_rate);
    builder.setFramesPerDataCallback(m_frame_size);
    builder.setChannelCount(ChannelCount::Mono);
    builder.setSampleRateConversionQuality(SampleRateConversionQuality::Medium);
    builder.setDataCallback(this);
    builder.setErrorCallback(this);
    Result result = builder.openStream(m_stream);
    log_i("%s(): %s", __FUNCTION__, "try to open stream");
    return result == Result::OK;
}