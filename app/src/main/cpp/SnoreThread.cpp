#include <cassert>
#include <snore.h>
#include "log.h"
#include "utils.h"
#include "global.h"
#include "SnoreThread.h"

using std::mutex;
using std::thread;
using std::unique_lock;
using std::condition_variable;
using snore::SNORE_I16pcm;
using snore::SNORE_F64pcm;
using snore::SNORE_ModelResult;
using snore::SNORE_PatientModel;
using snore::reduceNoise;
using snore::calculateModelResult;
using snore::generateNoiseProfile;
using snore::newModelResult;
using snore::newPatientModel;
using snore::deleteModelResult;
using snore::deletePatientModel;

TAG(SnoreThread)

SnoreThread::SnoreThread(SnoreJNICallback *callback, PatientThread *patientThread) : mCallback(
        callback), mPatientThread(patientThread), mSize(SNORE_BUFFER_SIZE), mBuffer(SAMPLE_RATE,
                                                                                    SNORE_BUFFER_SIZE,
                                                                                    SNORE_PADDING_SIZE),
                                                                                     mThread([this] {
                                                                                         EnvHelper helper;
                                                                                         this->run(
                                                                                                 helper.getEnv());
                                                                                     }) {
    mSampleRate = SAMPLE_RATE;
    unique_lock<mutex> lock(mMutex);
    while (!mAlive) {
        mCond.notify_all();
        mCond.wait(lock);
    }
}

SnoreThread::~SnoreThread() {
    waitForExit();
}

void SnoreThread::onAudioCallbackAttach() {
}

void SnoreThread::onAudioDataStart(int64_t timestamp) {
    unique_lock<mutex> lock(mMutex);
    if (mExit || !mAlive) {
        mCond.notify_all();
        return;
    }
    mState = DispatchState::START;
    mStart = timestamp;
    mSampleCount = 0;
    mFrame = 0;
    mBuffer.clear();
    mCond.notify_all();
}

void SnoreThread::onAudioDataStop(int64_t timestamp) {
    unique_lock<mutex> lock(mMutex);
    if (mExit || !mAlive) {
        mCond.notify_all();
        return;
    }
    mState = DispatchState::STOP;
    mStop = timestamp;
    mCond.notify_all();
}

void SnoreThread::onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) {
    unique_lock<mutex> lock(mMutex);
    if (!mAlive || mExit) {
        mCond.notify_all();
        return;
    }
    int32_t write = mBuffer.put(timestamp, data, length);
    mSampleCount += write;
    if (write != length) {
        log_w("%s(): discard %d samples", __FUNCTION__, length - write);
    }
    if (mBuffer.ready()) {
        mCond.notify_all();
    }
}

void SnoreThread::onAudioCallbackDetach() {
}

void SnoreThread::run(JNIEnv *env) {
    // 初始化
    unique_lock<mutex> lock(mMutex);
    mAlive = true;
    mCond.notify_all();
    lock.unlock();

    SnoreJNICallback *callback = mCallback;
    int64_t audioStartTime = 0, audioStopTime = 0, frameTimestamp = 0;
    DispatchState state = DispatchState::STOP;
    int32_t size = mSize;
    int32_t rd = 0;
    int32_t frame = 0;
    bool exit, ready = false, onStart = false, onStop = false;
    auto buf1 = new int16_t[size];
    auto buf2 = new int16_t[size];
    auto buf3 = new double[size];
    char s_prof[256];
    char s_temp[256];
    char s_audio[256];
    checkAndMkdir(g_external_base, g_audio);

    sprintf(s_prof, "%s/%s/%s", g_external_base, g_cache, "noise.prof");
    sprintf(s_temp, "%s/%s/%s", g_external_base, g_cache, "1min.wav");

    SNORE_PatientModel *patientModel = newPatientModel();

    while (true) {
        // sync state
        lock.lock();
        // 未退出，缓存池未准备好，状态未改变
        while (!mExit && !mBuffer.ready() && state == mState) {
            mCond.notify_all();
            mCond.wait(lock);
        }
        exit = mExit;
        if (state != mState && mState == DispatchState::START) {
            audioStartTime = mStart;
            onStart = true;
            frame = 0;
        }

        if (state != mState && mState == DispatchState::STOP) {
            audioStopTime = mStop;
            onStop = true;
        }

        if (mBuffer.ready()) {
            rd = mBuffer.next(buf1, size, frameTimestamp);
            log_d("%s(): timestamp = %lld", __FUNCTION__, frameTimestamp);
            ready = true;
        }
        state = mState;
        mCond.notify_all();
        lock.unlock();
        // do work
        if (exit) {
            break;
        }

        if (onStart) {
            onStart = false;
            callback->onStart(env, audioStartTime);
            //创建录音目录
            char dirname[64];
            sprintTimeMillis(dirname, audioStartTime);
            //去掉不支持的字符
            for (int32_t j = 0; dirname[j] != 0; ++j) {
                if (dirname[j] == ':' || dirname[j] == '.' || dirname[j] == ' ') {
                    dirname[j] = '-';
                }
            }
            sprintf(s_audio, "%s/%s", g_audio, dirname);
            checkAndMkdir(g_external_base, s_audio);
            sprintf(s_audio, "%s/%s/%s", g_external_base, g_audio, dirname);
            //清除之前存储数据
            if (patientModel != nullptr) {
                patientModel->clear();
            } else {
                patientModel = newPatientModel();
            }
        }

        if (ready) {
            ready = false;
            // 计算工作
            SNORE_I16pcm src = {buf1, (uint32_t) rd, 1, (double) mSampleRate};
            SNORE_I16pcm dst = {buf2, (uint32_t) rd, 1, (double) mSampleRate};
            SNORE_F64pcm fds = {buf3, SNORE_INPUT_SIZE, 1, (double) mSampleRate};
            if (frame == 0) {
                log_i("%s(): generate initial noise profile", __FUNCTION__);
                generateNoiseProfile(src, 0.0, 1.0, s_prof);
            }
            reduceNoise(src, dst, s_prof, 0.21);
            //save wav file before crash
            writeWav(s_temp, dst.raw, dst.length, 1, 44100);
            for (uint32_t i = 0; i < SNORE_INPUT_SIZE; ++i) {
                fds.raw[i] = dst.raw[i] / 32768.0;
            }
            SNORE_ModelResult *snoreResult = newModelResult();
            calculateModelResult(fds, *snoreResult);
            for (int64_t i = 0; i < snoreResult->getSignalIndexSize(); ++i) {
                int64_t startIndex = snoreResult->getSignalStart(i);
                int64_t endIndex = snoreResult->getSignalEnd(i);
                int64_t length = endIndex - startIndex;
                int64_t sms = (startIndex * 1000L) / mSampleRate;
                int64_t ems = (endIndex * 1000L) / mSampleRate;
                if (snoreResult->getSignalLabel(i) > 0.5) {
//                    log_i("%s(): frame: %d, audioStartTime: %lld ms, end: %lld ms, snoreResult: %s", __FUNCTION__,
//                          frame, sms, ems, snoreResult.label[i] > 0.5 ? "positive" : "negative");
                    int64_t snoreTimestamp = frameTimestamp + sms;
                    Snore snore = {snoreTimestamp, ems - sms, snoreResult->getSignalLabel(i) > 0.5,
                                   audioStartTime};
                    //保存鼾声录音
                    char filename[64];
                    sprintTimeMillis(filename, snoreTimestamp);
                    //去掉不支持的字符
                    for (int32_t j = 0; filename[j] != 0; ++j) {
                        if (filename[j] == ':' || filename[j] == '.' || filename[j] == ' ') {
                            filename[j] = '-';
                        }
                    }
                    strcat(filename, ".wav");
                    char filepath[256];
                    sprintf(filepath, "%s/%s", s_audio, filename);
                    callback->onRecognize(env, snore);
                    writeWav(filepath, &buf2[startIndex], length, 1, mSampleRate);
                    SNORE_F64pcm sig = {&fds.raw[snoreResult->getSignalStart(i)], length,
                                        fds.channel, fds.fs};
                    patientModel->digest(sig);
                }
            }
            if (snoreResult->getNoiseStart() >= 0 && snoreResult->getNoiseEnd() >= 0) {
                log_i("%s(): update noise profile", __FUNCTION__);
                generateNoiseProfile(src, snoreResult->getNoiseStart(), snoreResult->getNoiseEnd(),
                                     s_prof);
            }
            deleteModelResult(snoreResult);
            frame += 1;
        }

        if (onStop) {
            onStop = false;
            callback->onStop(env, audioStopTime);
            //提交数据
            if (patientModel != nullptr) {
                log_i("%s(): submit task", __FUNCTION__);
                if (mPatientThread->submitTask(audioStartTime, patientModel)) {
                    patientModel = newPatientModel();
                } else {
                    patientModel->clear();
                }
            } else {
                patientModel = newPatientModel();
            }
        }
    }

    delete[] buf1;
    delete[] buf2;
    delete[] buf3;
    deletePatientModel(patientModel);
    lock.lock();
    mAlive = false;
    mCond.notify_all();
    lock.unlock();
}

void SnoreThread::waitForExit() {
    unique_lock<mutex> lock(mMutex);
    if (!mAlive) return;
    mExit = true;
    while (mAlive) {
        mCond.notify_all();
        mCond.wait(lock);
    }
    lock.unlock();
    mThread.join();
}