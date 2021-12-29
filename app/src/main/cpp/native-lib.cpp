#include <jni.h>
#include <cassert>
#include "log.h"
#include "global.h"

static const char *const TAG = __FILE__;

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *env = nullptr;
    g_jvm = jvm;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) == JNI_OK) {
        LOG_D(TAG, "%s(): jni version 1.6", __FUNCTION__);
        g_version = JNI_VERSION_1_6;
        return JNI_VERSION_1_6;
    }
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) == JNI_OK) {
        LOG_D(TAG, "%s(): jni version 1.4", __FUNCTION__);
        g_version = JNI_VERSION_1_4;
        return JNI_VERSION_1_4;
    }
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_2) == JNI_OK) {
        LOG_D(TAG, "%s(): jni version 1.2", __FUNCTION__);
        g_version = JNI_VERSION_1_2;
        return JNI_VERSION_1_2;
    }
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_1) == JNI_OK) {
        LOG_D(TAG, "%s(): jni version 1.1", __FUNCTION__);
        g_version = JNI_VERSION_1_1;
        return JNI_VERSION_1_1;
    }
    LOG_D(__FILE__, "JNI_OnLoad(): cannot get m_env");
    return -1;
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_SnoringRecognition_nativeStart(JNIEnv *env, jobject thiz) {
    if (g_snoringRecognition == nullptr) {
        g_snoringRecognition = new SnoringRecognition(env, thiz);
    }
    //TODO set args
    return g_snoringRecognition->start();
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_SnoringRecognition_nativeStop(JNIEnv *env, jobject thiz) {
    if (g_snoringRecognition == nullptr) {
        g_snoringRecognition = new SnoringRecognition(env, thiz);
    }
    return g_snoringRecognition->stop();
}

JNIEXPORT jdouble JNICALL
Java_com_scut_utils_SnoringRecognition_nativeGetSampleRate(JNIEnv *env, jobject thiz) {
    if (g_snoringRecognition == nullptr) {
        g_snoringRecognition = new SnoringRecognition(env, thiz);
    }
    return g_snoringRecognition->getSampleRate();
}

JNIEXPORT jlong JNICALL
Java_com_scut_utils_SnoringRecognition_nativeGetStartTime(JNIEnv *env, jobject thiz) {
    if (g_snoringRecognition == nullptr) {
        g_snoringRecognition = new SnoringRecognition(env, thiz);
    }
    return g_snoringRecognition->getStartTime();
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_SnoringRecognition_nativeIsRunning(JNIEnv *env, jobject thiz) {
    if (g_snoringRecognition == nullptr) {
        g_snoringRecognition = new SnoringRecognition(env, thiz);
    }
    return g_snoringRecognition->isRunning();
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved) {
    JNIEnv *env;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) == JNI_OK) {
        LOG_D(TAG, "%s(): jni version 1.6", __FUNCTION__);
    } else if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) == JNI_OK) {
        LOG_D(TAG, "%s(): jni version 1.4", __FUNCTION__);
    } else if (jvm->GetEnv((void **) &env, JNI_VERSION_1_2) == JNI_OK) {
        LOG_D(TAG, "%s(): jni version 1.2", __FUNCTION__);
    } else if (jvm->GetEnv((void **) &env, JNI_VERSION_1_1) == JNI_OK) {
        LOG_D(TAG, "%s(): jni version 1.1", __FUNCTION__);
    } else {
        LOG_D(TAG, "%s(): cannot get env", __FUNCTION__);
    }
    delete g_snoringRecognition;
}

#ifdef __cplusplus
}
#endif