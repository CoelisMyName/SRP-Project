#include "log.h"
#include "utils.h"
#include "config.h"
#include "SnoreAudioRecord.h"

static const char *const TAG = "SnoreAudioRecord";

SnoreAudioRecord::SnoreAudioRecord(SnoreAudioCallback *cb) : m_running(false) {
    m_stream = nullptr;
    m_cb = cb;
}

SnoreAudioRecord::~SnoreAudioRecord() = default;

DataCallbackResult
SnoreAudioRecord::onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) {
    EnvHelper helper;
    JNIEnv *env = helper.getEnv();
    m_cb->put(env, (int16_t *) audioData, numFrames);
    return DataCallbackResult::Continue;
}

bool SnoreAudioRecord::onError(AudioStream *stream, Result result) {
    LOG_D(TAG, "onError(): %d", result);
    return AudioStreamErrorCallback::onError(stream, result);
}

bool SnoreAudioRecord::start() {
    if (m_running) {
        LOG_D(TAG, "start(): is already running");
        return true;
    }
    if (!openStream()) {
        LOG_D(TAG, "start(): cannot open input stream");
        return false;
    }
    Result result = m_stream->requestStart();
    if (result != Result::OK) {
        LOG_D(TAG, "start(): call m_stream->requestStart() failed");
        m_stream->close();
        m_stream.reset();
        return false;
    }
    m_running = true;
    LOG_D(TAG, "start(): success");
    return true;
}

bool SnoreAudioRecord::stop() {
    if (!m_running) {
        LOG_D(TAG, "stop(): has already stopped");
        return true;
    }
    m_stream->stop();
    m_stream->close();
    m_stream.reset();
    m_running = false;
    LOG_D(TAG, "stop(): stream stopped");
    return true;
}


bool SnoreAudioRecord::isRunning() {
    return m_running;
}

bool SnoreAudioRecord::openStream() {
    AudioStreamBuilder builder;
    builder.setFormat(AudioFormat::I16);
    builder.setDirection(Direction::Input);
    builder.setFormatConversionAllowed(true);
    builder.setSharingMode(SharingMode::Exclusive);
    //TODO
    builder.setSampleRate(SAMPLE_RATE);
    builder.setFramesPerDataCallback(FRAME_SIZE);
    builder.setChannelCount(ChannelCount::Mono);
    builder.setSampleRateConversionQuality(SampleRateConversionQuality::Medium);
    builder.setDataCallback(this);
    builder.setErrorCallback(this);
    Result result = builder.openStream(m_stream);
    return result == Result::OK;
}