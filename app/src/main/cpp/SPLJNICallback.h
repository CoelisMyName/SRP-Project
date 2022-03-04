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
    SPLJNICallback(JNIEnv *env, jobject obj) : mBuilder(env) {
        mEnv = env;
        mObj = env->NewGlobalRef(obj);
        jclass local_class = env->FindClass("com/scut/utils/ModuleController");
        mCls = (jclass) env->NewGlobalRef(local_class);
        env->DeleteLocalRef(local_class);
        mOnStart = env->GetMethodID(mCls, "onSPLStart", "(J)V");
        mOnDetect = env->GetMethodID(mCls, "onSPLDetect", "(Lcom/scut/utils/SPL;)V");
        mOnStop = env->GetMethodID(mCls, "onSPLStop", "(J)V");
    }

    /**
     * 当需要回收对象时更新 env
     * @param env
     */
    void updateEnv(JNIEnv *env) {
        mEnv = env;
        mBuilder.updateEnv(env);
    }

    ~SPLJNICallback() {
        mEnv->DeleteGlobalRef(mObj);
        mEnv->DeleteGlobalRef(mCls);
    }

    // java 方法
    void onStart(JNIEnv *env, jlong timestamp) {
        env->CallVoidMethod(mObj, mOnStart, timestamp);
    }

    void onDetect(JNIEnv *env, SPL &spl) {
        jobject obj = mBuilder.getNewSPL(env, spl);
        env->CallVoidMethod(mObj, mOnDetect, obj);
        env->DeleteLocalRef(obj);
    }

    void onStop(JNIEnv *env, jlong timestamp) {
        env->CallVoidMethod(mObj, mOnStop, timestamp);
    }

private:
    JNIEnv *mEnv;
    jobject mObj;
    jclass mCls;
    SPLBuilder mBuilder;
    jmethodID mOnStart;
    jmethodID mOnDetect;
    jmethodID mOnStop;
};

#endif