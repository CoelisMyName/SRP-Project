#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include "log.h"
#include "utils.h"
#include "global.h"
#include "MyMLLiveModel.h"

static const char *const TAG = "MyMLLiveModel";

using std::unique_lock;

DataCallbackResult
MyMLLiveModel::onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) {
    const int16_t *input = (int16_t *) audioData;
    frameProcess(input, numFrames);
    return DataCallbackResult::Continue;
}

bool MyMLLiveModel::onError(AudioStream *stream, Result result) {
    LOG_D(TAG, "onError(): %d", result);
    return AudioStreamErrorCallback::onError(stream, result);
}

MyMLLiveModel::MyMLLiveModel() {
    m_args = {false, false, nullptr};
    m_bufferA = new I16Buffer{new int16_t[BUFFER_SIZE], BUFFER_SIZE, 0};
    m_bufferB = new I16Buffer{new int16_t[BUFFER_SIZE], BUFFER_SIZE, 0};
}

MyMLLiveModel::~MyMLLiveModel() {
    shutdown();
    delete[] m_buffer;
    delete[] m_bufferA->buffer;
    delete[] m_bufferB->buffer;
    delete m_bufferA;
    delete m_bufferB;
}

bool MyMLLiveModel::start() {
    if (!m_initial_success) {
        LOG_D(TAG, "start(): call start before initial");
        return false;
    }
    if (m_running) {
        LOG_D(TAG, "start(): is already running");
        return true;
    }
    if (!m_threadFinished) {
        LOG_D(TAG, "start(): is not yet finished");
        return false;
    }
    if (!openStream()) {
        LOG_D(TAG, "start(): cannot open input stream");
        return false;
    }
    Result result = m_stream->requestStart();
    m_clock = clock();
    if (result != Result::OK) {
        LOG_D(TAG, "start(): call m_stream->requestStart() failed");
        m_stream->close();
        m_stream.reset();
        return false;
    }
    m_worker = new thread([this] { this->run(); });
    m_threadFinished = false;
    m_running = true;
    return true;
}

bool MyMLLiveModel::stop() {
    if (!m_running) {
        LOG_D(TAG, "stop(): is already stop");
        return true;
    }
    m_stream->stop();
    m_stream->close();
    unique_lock<mutex> lock(m_mutex);
    m_cond.notify_all();
    lock.unlock();
    reset();
    LOG_D(TAG, "stop(): stream stopped");
    return true;
}

bool MyMLLiveModel::openStream() {
    AudioStreamBuilder builder;
    builder.setFormat(AudioFormat::I16);
    builder.setDirection(Direction::Input);
    builder.setFormatConversionAllowed(true);
    builder.setSharingMode(SharingMode::Exclusive);
    builder.setSampleRate(SAMPLE_RATE);
    builder.setFramesPerDataCallback(FRAME_SIZE);
    builder.setChannelCount(ChannelCount::Mono);
    builder.setSampleRateConversionQuality(SampleRateConversionQuality::Medium);
    builder.setDataCallback(this);
    builder.setErrorCallback(this);
    Result result = builder.openStream(m_stream);
    return result == Result::OK;
}

void MyMLLiveModel::frameProcess(const int16_t *raw, uint32_t length) {
    clock_t start, end;
    start = clock();
    JNIEnv *env = nullptr;
    uint32_t write = 0, remain = length;
    while (remain > 0) {
        uint32_t tmp = copyToBuffer(&raw[write], remain);
        if (tmp < remain) {
            swap();
            m_cond.notify_all();
        }
        remain -= tmp;
        write += tmp;
    }
    for (int i = 0; i < length && i < FRAME_SIZE; ++i) {
        m_buffer[i] = raw[i] / 32768.0;
    }

    F64pcm pcm = {m_buffer, length, 1, SAMPLE_RATE};
    SPL spl;
    calculateSPL(pcm, spl);

    g_jvm->AttachCurrentThread(&env, nullptr);
    jdoubleArray freq, a_pow, c_pow, z_pow;
    freq = env->NewDoubleArray(8);
    a_pow = env->NewDoubleArray(8);
    c_pow = env->NewDoubleArray(8);
    z_pow = env->NewDoubleArray(8);
    env->SetDoubleArrayRegion(freq, 0, 8, spl.freq);
    env->SetDoubleArrayRegion(a_pow, 0, 8, spl.a_pow);
    env->SetDoubleArrayRegion(c_pow, 0, 8, spl.c_pow);
    env->SetDoubleArrayRegion(z_pow, 0, 8, spl.z_pow);
    jlong timestamp = clock();
    jobject jo_spl = env->NewObject(m_spl_class, m_spl_init, timestamp, (jdouble) spl.a_sum,
                                    (jdouble) spl.c_sum, (jdouble) spl.z_sum, freq, a_pow, c_pow,
                                    z_pow);
    env->CallVoidMethod(m_this, m_splCallback, jo_spl);
    g_jvm->DetachCurrentThread();

    end = clock();
#ifdef NATIVE_DEBUG
    LOG_D(TAG, "frameProcess(): time cost %ld(ms)", end - start);
    if (end - start >= 100)
        LOG_W(TAG, "frameProcess(): warning time cost above 100(ms)");
#endif
}

void MyMLLiveModel::run() {
    JNIEnv *env;
    g_jvm->AttachCurrentThread(&env, nullptr);
    LOG_D(TAG, "run(): thread in place");

    MyMLLiveModelArgs args = m_args;
    char s_folder[2048];
    strcpy(s_folder, g_external_path);
    if (args.savePerMinute || args.savePositive) {
        if (args.saveFolder != nullptr && strlen(args.saveFolder) > 0) {
            strcat(s_folder, "/");
            strcat(s_folder, args.saveFolder);
        }
    }

    char s_prof[2048];
    strcpy(s_prof, g_external_path);
    strcat(s_prof, "/noiseprof.txt");

    I16pcm dst = {(int16_t *) malloc(sizeof(int16_t) * BUFFER_SIZE), BUFFER_SIZE, 1, SAMPLE_RATE};

    while (true) {
        unique_lock<mutex> lock(m_mutex);
        if (!m_running) break;
        m_cond.wait(lock);
        LOG_D(TAG, "run(): thread awake");
        if (!m_running) break;
        lock.unlock();
        LOG_D(TAG, "run(): thread is working");
        I16pcm src = {m_bufferB->buffer, m_bufferB->capacity, 1, SAMPLE_RATE};
        if (m_minute <= 1) {
            LOG_D(TAG, "run(): initial noise profile");
            generateNoiseProfile(src, 0, 1, s_prof);
        }

        reduceNoise(src, dst, s_prof, 0.21);
        F64pcm fpcm;
        convert(dst, fpcm);
        ModelResult result;
        calculateModelResult(fpcm, result);
        generateNoiseProfile(src, result.n_start, result.n_length, s_prof);
        callClsCallback(env, result);
        free(fpcm.raw);

        if (args.savePerMinute) {
            //TODO save src dst
        }
        if (args.savePositive) {
            //TODO save positive
        }
        if (!m_running) break;
    }
    LOG_D(TAG, "run(): thread exiting");
    m_threadFinished = true;
    g_jvm->DetachCurrentThread();
}

void MyMLLiveModel::reset() {
    m_running = false;
    m_bufferA->position = 0;
    m_bufferB->position = 0;
    m_clock = 0;
    m_minute = 0;
    m_stream.reset();
    m_worker->detach();
    delete m_worker;
    m_worker = nullptr;
}

uint32_t MyMLLiveModel::copyToBuffer(const int16_t *raw, uint32_t len) {
    volatile I16Buffer *wrt = m_bufferA;
    uint32_t wrtLen = fmin(wrt->capacity - wrt->position, len);
    memcpy(&wrt->buffer[wrt->position], raw, wrtLen * sizeof(int16_t));
    wrt->position += wrtLen;
    return wrtLen;
}

void MyMLLiveModel::shutdown() {
    stop();
    unique_lock<mutex> lock(m_mutex);
    m_shut = true;
    m_cond.notify_all();
    lock.unlock();
    m_worker->join();
    delete m_worker;
    m_worker = nullptr;
}

void MyMLLiveModel::swap() {
    //拷贝最后一段音频到bufferB
    memcpy(m_bufferB->buffer, &m_bufferA->buffer[INPUT_SIZE],
           sizeof(int16_t) * (BUFFER_SIZE - INPUT_SIZE));
    m_bufferB->position = BUFFER_SIZE - INPUT_SIZE;
    auto *tmp = (I16Buffer *) m_bufferA;
    m_bufferA = m_bufferB;
    m_bufferB = tmp;
    m_minute++;
}

bool MyMLLiveModel::jniInitial(JNIEnv *env, jobject thiz) {
    if (strcmp(g_external_path, "null") == 0) {
        LOG_D(TAG, "external path is not initialized");
        m_initial_success = false;
        return false;
    }
    if (m_initial_success) return true;
    m_this = env->NewGlobalRef(thiz);
    m_class = (jclass) env->NewGlobalRef(env->FindClass("com/scut/utils/MyMLLiveModel"));
    m_spl_class = (jclass) env->NewGlobalRef(env->FindClass("com/scut/utils/SPL"));
    m_splCallback = env->GetMethodID(m_class, "calculateSPL", "(Lcom/scut/utils/SPL;)V");
    m_clsCallback = env->GetMethodID(m_class, "calculateCLS", "(JJJZ)V");
    m_spl_init = env->GetMethodID(m_spl_class, "<init>", "(JDDD[D[D[D[D)V");
    m_initial_success = true;
    return true;
}

bool MyMLLiveModel::jniDestroy(JNIEnv *env) {
    if (!m_initial_success) return true;
    env->DeleteGlobalRef(m_this);
    env->DeleteGlobalRef(m_class);
    env->DeleteGlobalRef(m_spl_class);
    m_initial_success = false;
    return true;
}

void MyMLLiveModel::setArgs(MyMLLiveModelArgs &args) {
    m_args = args;
}

void MyMLLiveModel::clearArgs(MyMLLiveModelArgs &args) {
    args = m_args;
    args = {false, false, nullptr};
}

void MyMLLiveModel::callClsCallback(JNIEnv *env, ModelResult &result) {
    jlong minute = m_minute - 1, start, end;
    jboolean pos;
    int size = fmin(result.s_size, fmin(result.e_size, result.l_size));
    for (int i = 0; i < size; ++i) {
        start = result.starts[i];
        end = result.ends[i];
        pos = result.label[i] > 0.5;
        env->CallVoidMethod(m_this, m_clsCallback, minute, start, end, pos);
    }
}


