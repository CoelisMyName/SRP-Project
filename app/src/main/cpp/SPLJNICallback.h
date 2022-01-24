#ifndef SRP_PROJECT_SPLJNICALLBACK_H
#define SRP_PROJECT_SPLJNICALLBACK_H

#include <jni.h>
#include "SPLBuilder.h"

/**
 *
 */
class SPLJNICallback {
public:
    /**
     * 该类最好在全局下保存
     * 只能在主线程调用
     * @param env 主线程的 env
     * @param obj ModuleController 实例
     */
    SPLJNICallback(JNIEnv *env, jobject obj) : m_builder(env) {
        m_env = env;
        m_obj = env->NewGlobalRef(obj);
        jclass local_class = env->FindClass("com/scut/utils/ModuleController");
        m_cls = (jclass) env->NewGlobalRef(local_class);
        env->DeleteLocalRef(local_class);
        m_onStart = env->GetMethodID(m_cls, "onSPLStart", "(J)V");
        m_onDetect = env->GetMethodID(m_cls, "onSPLDetect", "(Lcom/scut/utils/SPL;)V");
        m_onStop = env->GetMethodID(m_cls, "onSPLStop", "(J)V");
    }

    /**
     * 当需要回收对象时更新 env
     * @param env
     */
    void updateEnv(JNIEnv *env) {
        m_env = env;
    }

    ~SPLJNICallback() {
        m_env->DeleteGlobalRef(m_obj);
        m_env->DeleteGlobalRef(m_cls);
    }

    // java 方法
    void onStart(JNIEnv *env, jlong timestamp) {
        env->CallVoidMethod(m_obj, m_onStart, timestamp);
    }

    void onDetect(JNIEnv *env, SPL &spl) {
        jobject obj = m_builder.getNewSPL(env, spl);
        env->CallVoidMethod(m_obj, m_onDetect, obj);
        env->DeleteLocalRef(obj);
    }

    void onStop(JNIEnv *env, jlong timestamp) {
        env->CallVoidMethod(m_obj, m_onStop, timestamp);
    }

private:
    JNIEnv *m_env;
    jobject m_obj;
    jclass m_cls;
    SPLBuilder m_builder;
    jmethodID m_onStart;
    jmethodID m_onDetect;
    jmethodID m_onStop;
};

#endif