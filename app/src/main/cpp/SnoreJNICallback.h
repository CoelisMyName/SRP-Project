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
        mEnv = env;
        // 全局共享引用
        mObj = env->NewGlobalRef(obj);
        jclass local_class = env->FindClass("com/scut/utils/ModuleController");
        mCls = (jclass) env->NewGlobalRef(local_class);
        env->DeleteLocalRef(local_class);
        local_class = env->FindClass("com/scut/utils/Snore");
        mSnore = (jclass) env->NewGlobalRef(local_class);
        env->DeleteLocalRef(local_class);
        // 方法 id
        mConstructor = env->GetMethodID(mSnore, "<init>", "(JJZJ)V");
        mOnStart = env->GetMethodID(mCls, "onSnoreStart", "(J)V");
        mOnRecognize = env->GetMethodID(mCls, "onSnoreRecognize", "(Lcom/scut/utils/Snore;)V");
        mOnStop = env->GetMethodID(mCls, "onSnoreStop", "(J)V");
    }

    /**
     * 当需要回收对象时更新 env
     * @param env
     */
    void updateEnv(JNIEnv *env) {
        mEnv = env;
    }

    ~SnoreJNICallback() {
        mEnv->DeleteGlobalRef(mCls);
        mEnv->DeleteGlobalRef(mSnore);
    }

    void onStart(JNIEnv *env, jlong timestamp) {
        env->CallVoidMethod(mObj, mOnStart, timestamp);
    }

    void onRecognize(JNIEnv *env, Snore &snore) {
        jobject obj = env->NewObject(mSnore, mConstructor, snore.timestamp, snore.length,
                                     snore.confirm, snore.startTime);
        env->CallVoidMethod(mObj, mOnRecognize, obj);
        env->DeleteLocalRef(obj);
    }

    void onStop(JNIEnv *env, jlong timestamp) {
        env->CallVoidMethod(mObj, mOnStop, timestamp);
    }

private:
    JNIEnv *mEnv;
    // 全局引用类型
    jobject mObj;
    jclass mCls;
    jclass mSnore;
    jmethodID mOnStart;
    jmethodID mOnRecognize;
    jmethodID mOnStop;
    jmethodID mConstructor;
};


#endif