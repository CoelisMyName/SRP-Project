#ifndef SRP_PROJECT_SNOREJNICALLBACK_H
#define SRP_PROJECT_SNOREJNICALLBACK_H

#include <jni.h>

typedef struct {
    jlong timestamp;
    jlong length;
    jboolean confirm;
    jlong startTime;
} Snore;

/**
 * 线程不安全，因为持有 JNIEnv 指针，该方法对应 utils.ModuleController 下的方法
 */
class SnoreJNICallback {
public:
    SnoreJNICallback(JNIEnv *env, jobject obj) {
        m_env = env;
        m_obj = obj;
        m_cls = env->FindClass("com/scut/utils/ModuleController");
        m_snore = env->FindClass("com/scut/utils/Snore");
        m_constructor = env->GetMethodID(m_snore, "<init>", "(JJZJ)V");
        m_onStart = env->GetMethodID(m_cls, "onSnoreStart", "(J)V");
        m_onRecognize = env->GetMethodID(m_cls, "onSnoreRecognize", "(Lcom/scut/utils/Snore;)V");
        m_onStop = env->GetMethodID(m_cls, "onSnoreStop", "(J)V");
    }

    ~SnoreJNICallback() {
        m_env->DeleteLocalRef(m_cls);
    }

    void onStart(jlong timestamp) {
        m_env->CallVoidMethod(m_obj, m_onStart, timestamp);
    }

    void onRecognize(Snore &snore) {
        jobject obj = m_env->NewObject(m_snore, m_constructor, snore.timestamp, snore.length,
                                       snore.confirm, snore.startTime);
        m_env->CallVoidMethod(m_obj, m_onRecognize, obj);
        m_env->DeleteLocalRef(obj);
    }

    void onStop(jlong timestamp) {
        m_env->CallVoidMethod(m_obj, m_onStop, timestamp);
    }

private:
    JNIEnv *m_env;
    jobject m_obj;
    jclass m_cls;
    jclass m_snore;
    jmethodID m_onStart;
    jmethodID m_onRecognize;
    jmethodID m_onStop;
    jmethodID m_constructor;
};


#endif