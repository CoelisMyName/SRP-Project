#ifndef SRP_PROJECT_SNOREJNICALLBACK_H
#define SRP_PROJECT_SNOREJNICALLBACK_H

#include <jni.h>
#include <cstdint>

/**
 * JNI回调类，提供回调接口，可跨线程使用
 */
class SnoreJNICallback {
public:
    SnoreJNICallback(JNIEnv *env, jobject obj);

    ~SnoreJNICallback();

    void onRecognize(JNIEnv *env, uint64_t minute, uint64_t start, uint64_t end, bool positive);

    void onSPLDetect(JNIEnv *env, jobject spl);

private:
    jobject m_this;///SnoringRecognition实例
    jclass m_class;///SnoringRecognition类
    jmethodID m_splCallback, m_rcgCallback;///SnoringRecognition成员方法
};

#endif