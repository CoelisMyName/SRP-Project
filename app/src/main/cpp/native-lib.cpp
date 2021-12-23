#include <jni.h>
#include <cassert>
#include "log.h"
#include "global.h"

static const char *const TAG = "native-lib";

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *env = nullptr;
    g_jvm = jvm;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) == JNI_OK) {
        LOG_D(TAG, "JNI_OnLoad(): jni version 1.6");
        return JNI_VERSION_1_6;
    }
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) == JNI_OK) {
        LOG_D(TAG, "JNI_OnLoad(): jni version 1.4");
        return JNI_VERSION_1_4;
    }
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_2) == JNI_OK) {
        LOG_D(TAG, "JNI_OnLoad(): jni version 1.2");
        return JNI_VERSION_1_2;
    }
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_1) == JNI_OK) {
        LOG_D(TAG, "JNI_OnLoad(): jni version 1.1");
        return JNI_VERSION_1_1;
    }
    LOG_D(TAG, "JNI_OnLoad(): cannot get env");
    return -1;
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_MyMLLiveModel_start(JNIEnv *env, jobject thiz) {
    if (g_model == nullptr) return false;
    MyMLLiveModelArgs args;

    jfieldID jf_tmp;
    jstring js_tmp;
    jclass jc_tmp;
    jobject jo_tmp;
    const char *s_tmp;

    jc_tmp = env->GetObjectClass(thiz);
    jf_tmp = env->GetStaticFieldID(jc_tmp, "m_config", "Lcom/scut/utils/ModelConfig;");
    jo_tmp = env->GetStaticObjectField(jc_tmp, jf_tmp);
    jc_tmp = env->GetObjectClass(jo_tmp);
    jf_tmp = env->GetFieldID(jc_tmp, "savePerMinute", "Z");
    args.savePerMinute = env->GetBooleanField(jo_tmp, jf_tmp);
    jf_tmp = env->GetFieldID(jc_tmp, "savePositive", "Z");
    args.savePositive = env->GetBooleanField(jo_tmp, jf_tmp);
    jf_tmp = env->GetFieldID(jc_tmp, "saveFolder", "Ljava/lang/String;");
    js_tmp = (jstring) env->GetObjectField(jo_tmp, jf_tmp);
    s_tmp = env->GetStringUTFChars(js_tmp, nullptr);
    args.saveFolder = (char *) malloc(strlen(s_tmp) + 1);
    strcpy(args.saveFolder, s_tmp);
    env->ReleaseStringUTFChars(js_tmp, s_tmp);

    g_model->setArgs(args);
    return g_model->start();
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_MyMLLiveModel_stop(JNIEnv *env, jobject thiz) {
    if (g_model == nullptr) return false;
    bool res = g_model->stop();
    MyMLLiveModelArgs args = {false, false, nullptr};
    g_model->clearArgs(args);
    free(args.saveFolder);
    return res;
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_MyMLLiveModel_init(JNIEnv *env, jobject thiz) {
    jfieldID jf_tmp;
    jstring js_tmp;
    jclass jc_tmp;
    const char *s_tmp;
    //保存目录
    jc_tmp = env->GetObjectClass(thiz);
    jf_tmp = env->GetStaticFieldID(jc_tmp, "m_folder", "Ljava/lang/String;");
    js_tmp = (jstring) env->GetStaticObjectField(jc_tmp, jf_tmp);
    s_tmp = env->GetStringUTFChars(js_tmp, nullptr);
    strcpy(g_external_path, s_tmp);
    env->ReleaseStringUTFChars(js_tmp, s_tmp);
    if (g_model == nullptr) {
        g_model = new MyMLLiveModel();
    }
    return g_model->jniInitial(env, thiz);
//    return g_model->isInitialSuccess();
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved) {
    JNIEnv *env;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) == JNI_OK) {
        LOG_D(TAG, "JNI_OnUnload(): jni version 1.6");
        g_model->jniDestroy(env);
    } else if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) == JNI_OK) {
        LOG_D(TAG, "JNI_OnUnload(): jni version 1.4");
        g_model->jniDestroy(env);
    } else if (jvm->GetEnv((void **) &env, JNI_VERSION_1_2) == JNI_OK) {
        LOG_D(TAG, "JNI_OnUnload(): jni version 1.2");
        g_model->jniDestroy(env);
    } else if (jvm->GetEnv((void **) &env, JNI_VERSION_1_1) == JNI_OK) {
        LOG_D(TAG, "JNI_OnUnload(): jni version 1.1");
        g_model->jniDestroy(env);
    } else {
        LOG_D(TAG, "JNI_OnUnload(): cannot get env");
    }
    delete g_model;
}

#ifdef __cplusplus
}
#endif
