#ifndef SRP_PROJECT_MYMLLIVEMODEL_H
#define SRP_PROJECT_MYMLLIVEMODEL_H

#include <jni.h>
#include <time.h>
#include <thread>
#include <oboe/Oboe.h>
#include "model.h"
#include "config.h"

using namespace oboe;
using namespace model;
using std::shared_ptr;
using std::atomic;
using std::thread;
using std::mutex;
using std::condition_variable;

typedef struct I16Buffer {
    int16_t *buffer;
    uint32_t capacity;
    uint32_t position;
} I16Buffer;

typedef struct MyMLLiveModelArgs {
    bool savePerMinute, savePositive;
    char *saveFolder;

} MyMLLiveModelArgs;

class MyMLLiveModel : public oboe::AudioStreamCallback {
public:
    MyMLLiveModel();

    void setArgs(MyMLLiveModelArgs &args);

    void clearArgs(MyMLLiveModelArgs &args);

    virtual ~MyMLLiveModel();

    bool start();

    bool stop();

    void shutdown();

    DataCallbackResult
    onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) override;

    bool onError(AudioStream *stream, Result result) override;

    bool jniInitial(JNIEnv *env, jobject thiz);

    bool jniDestroy(JNIEnv *env);

    uint64_t getStartTime() {
        return m_clock;
    }

    bool isInitialSuccess() {
        return m_initial_success;
    }

private:
    //// 配置
    MyMLLiveModelArgs m_args{};
    //// 模型工作线程
    thread *m_worker{nullptr};
    //// 缓存
    volatile I16Buffer *m_bufferA, *m_bufferB;
    //// 开始时间戳
    clock_t m_clock = 0;

    shared_ptr<AudioStream> m_stream;

    atomic<bool> m_running{false};

    atomic<bool> m_threadFinished{true};

    atomic<uint64_t> m_minute{0};

    double *m_buffer = new double[FRAME_SIZE];

    mutex m_mutex;

    condition_variable m_cond;

    atomic<bool> m_shut{false};

    jobject m_this{};

    jclass m_class{}, m_spl_class{};

    jmethodID m_splCallback{}, m_clsCallback{}, m_spl_init{};

    bool m_initial_success = false;

    bool openStream();

    void frameProcess(const int16_t *raw, uint32_t length);

    uint32_t copyToBuffer(const int16_t *raw, uint32_t len);

    void reset();

    void swap();

    void run();

    void callClsCallback(JNIEnv *env, ModelResult &result);
};


#endif
