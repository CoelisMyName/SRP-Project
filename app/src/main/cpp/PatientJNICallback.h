#ifndef SRP_PROJECT_PATIENTJNICALLBACK_H
#define SRP_PROJECT_PATIENTJNICALLBACK_H

#include <jni.h>

class PatientJNICallback {
public:
    PatientJNICallback(JNIEnv *env, jobject obj) {
        mEnv = env;
        // 全局共享引用
        mObj = env->NewGlobalRef(obj);
        jclass local_class = env->FindClass("com/scut/utils/ModuleController");
        mCls = (jclass) env->NewGlobalRef(local_class);
        env->DeleteLocalRef(local_class);
        // 方法 id
        mOnPatientResult = env->GetMethodID(mCls, "onPatientResult", "(JD)V");
    }

    /**
     * 当需要回收对象时更新 env
     * @param env
     */
    void updateEnv(JNIEnv *env) {
        mEnv = env;
    }

    ~PatientJNICallback() {
        mEnv->DeleteGlobalRef(mObj);
        mEnv->DeleteGlobalRef(mCls);
    }

    void onPatientResult(JNIEnv *env, jlong timestamp, jdouble label) {
        env->CallVoidMethod(mObj, mOnPatientResult, timestamp, label);
    }

private:
    JNIEnv *mEnv;
    // 全局引用类型
    jobject mObj;
    jclass mCls;
    jmethodID mOnPatientResult;

};

#endif