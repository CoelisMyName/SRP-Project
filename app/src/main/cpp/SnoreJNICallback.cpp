#include "utils.h"
#include "SnoreJNICallback.h"

SnoreJNICallback::SnoreJNICallback(JNIEnv *env, jobject obj) {
    m_this = env->NewGlobalRef(obj);
    jclass jc_tmp = env->FindClass("com/scut/utils/SnoringRecognition");
    m_class = (jclass) env->NewGlobalRef(jc_tmp);
    env->DeleteLocalRef(jc_tmp);
    m_rcgCallback = env->GetMethodID(m_class, "onRecognize", "(JJJZ)V");
    m_splCallback = env->GetMethodID(m_class, "onSPLDetect", "(Lcom/scut/utils/SPL;)V");
}

SnoreJNICallback::~SnoreJNICallback() {
    EnvHelper helper;
    JNIEnv *env = helper.getEnv();
    env->DeleteGlobalRef(m_this);
    env->DeleteGlobalRef(m_class);
}

void SnoreJNICallback::onRecognize(JNIEnv *env, uint64_t minute, uint64_t start, uint64_t end,
                                   bool positive) {
    jlong j_min = minute;
    jlong j_start = start;
    jlong j_end = end;
    jboolean j_pos = positive;
    env->CallVoidMethod(m_this, m_rcgCallback, j_min, j_start, j_end, j_pos);
}

void SnoreJNICallback::onSPLDetect(JNIEnv *env, jobject spl) {
    env->CallVoidMethod(m_this, m_splCallback, spl);
}
