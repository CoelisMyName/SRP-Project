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
        // 全局共享引用
        m_obj = env->NewGlobalRef(obj);
        jclass local_class = env->FindClass("com/scut/utils/ModuleController");
        m_cls = (jclass) env->NewGlobalRef(local_class);
        env->DeleteLocalRef(local_class);
        local_class = env->FindClass("com/scut/utils/Snore");
        m_snore = (jclass) env->NewGlobalRef(local_class);
        env->DeleteLocalRef(local_class);
        // 方法 id
        m_constructor = env->GetMethodID(m_snore, "<init>", "(JJZJ)V");
        m_onStart = env->GetMethodID(m_cls, "onSnoreStart", "(J)V");
        m_onRecognize = env->GetMethodID(m_cls, "onSnoreRecognize", "(Lcom/scut/utils/Snore;)V");
        m_onStop = env->GetMethodID(m_cls, "onSnoreStop", "(J)V");
    }

    /**
     * 当需要回收对象时更新 env
     * @param env
     */
    void updateEnv(JNIEnv *env) {
        m_env = env;
    }

    ~SnoreJNICallback() {
        m_env->DeleteGlobalRef(m_cls);
        m_env->DeleteGlobalRef(m_snore);
    }

    void onStart(JNIEnv *env, jlong timestamp) {
        env->CallVoidMethod(m_obj, m_onStart, timestamp);
    }

    void onRecognize(JNIEnv *env, Snore &snore) {
        jobject obj = env->NewObject(m_snore, m_constructor, snore.timestamp, snore.length,
                                     snore.confirm, snore.startTime);
        env->CallVoidMethod(m_obj, m_onRecognize, obj);
        env->DeleteLocalRef(obj);
    }

    void onStop(JNIEnv *env, jlong timestamp) {
        env->CallVoidMethod(m_obj, m_onStop, timestamp);
    }

private:
    JNIEnv *m_env;
    // 全局引用类型
    jobject m_obj;
    jclass m_cls;
    jclass m_snore;
    jmethodID m_onStart;
    jmethodID m_onRecognize;
    jmethodID m_onStop;
    jmethodID m_constructor;
};


#endif