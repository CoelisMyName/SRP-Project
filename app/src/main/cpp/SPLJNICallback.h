#ifndef SRP_PROJECT_SPLJNICALLBACK_H
#define SRP_PROJECT_SPLJNICALLBACK_H

#include <jni.h>
#include "SPLBuilder.h"

class SPLJNICallback {
public:
    SPLJNICallback(JNIEnv *env, jobject obj) : m_builder(env) {
        m_env = env;
        m_obj = obj;
        m_cls = env->FindClass("com/scut/utils/ModuleController");
        m_onStart = env->GetMethodID(m_cls, "onSPLStart", "(J)V");
        m_onDetect = env->GetMethodID(m_cls, "onSPLDetect", "(Lcom/scut/utils/SPL;)V");
        m_onStop = env->GetMethodID(m_cls, "onSPLStop", "(J)V");
    }

    ~SPLJNICallback() {
        m_env->DeleteLocalRef(m_cls);
    }

    void onStart(jlong timestamp) {
        m_env->CallVoidMethod(m_obj, m_onStart, timestamp);
    }

    void onDetect(SPL &spl) {
        jobject obj = m_builder.getNewSPL(spl);
        m_env->CallVoidMethod(m_obj, m_onDetect, obj);
        m_env->DeleteLocalRef(obj);
    }

    void onStop(jlong timestamp) {
        m_env->CallVoidMethod(m_obj, m_onStop, timestamp);
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