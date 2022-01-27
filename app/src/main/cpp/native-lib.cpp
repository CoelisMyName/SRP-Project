#include <jni.h>
#include <cassert>
#include "log.h"
#include "global.h"
#include "GLThread.h"
#include "SPLThread.h"
#include "SnoreThread.h"
#include "AudioSource.h"
#include "AudioRecord.h"
#include "RenderFactory.h"
#include "SPLJNICallback.h"
#include "SnoreJNICallback.h"
#include "AudioDataDispatcher.h"

TAG(libsrp)

static AudioSource *audioSource = nullptr;
static AudioDataDispatcher *dispatcher = nullptr;
static SnoreThread *snoreThread = nullptr;
static SPLThread *splThread = nullptr;
static SnoreJNICallback *snoreJNICallback = nullptr;
static SPLJNICallback *splJNICallback = nullptr;
static bool initialFlag = false;

extern "C" {
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *env = nullptr;
    g_jvm = jvm;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) == JNI_OK) {
        g_version = JNI_VERSION_1_6;
        return JNI_VERSION_1_6;
    }
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) == JNI_OK) {
        g_version = JNI_VERSION_1_4;
        return JNI_VERSION_1_4;
    }
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_2) == JNI_OK) {
        g_version = JNI_VERSION_1_2;
        return JNI_VERSION_1_2;
    }
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_1) == JNI_OK) {
        g_version = JNI_VERSION_1_1;
        return JNI_VERSION_1_1;
    }
    return -1;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved) {
    JNIEnv *env;
    jvm->GetEnv((void **) &env, g_version);
}
// LibGLThread.kt
JNIEXPORT jlong JNICALL
Java_com_scut_component_LibGLThread_create(JNIEnv *env, jobject thiz, jobject view,
                                           jlong render) {
    auto p_render = (GLRender *) render;
    auto p_thread = new GLThread(env, view, p_render);
    auto thread = (jlong) p_thread;
    return thread;
}

JNIEXPORT void JNICALL
Java_com_scut_component_LibGLThread_surfaceCreate(JNIEnv *env, jobject thiz,
                                                  jlong thread, jobject surface,
                                                  jint width, jint height) {
    auto p_thread = (GLThread *) thread;
    p_thread->surfaceCreate(env, surface, width, height);
}

JNIEXPORT void JNICALL
Java_com_scut_component_LibGLThread_surfaceSizeChanged(JNIEnv *env, jobject thiz,
                                                       jlong thread, jobject surface,
                                                       jint width, jint height) {
    auto p_thread = (GLThread *) thread;
    p_thread->surfaceSizeChanged(env, surface, width, height);
}

JNIEXPORT jboolean JNICALL
Java_com_scut_component_LibGLThread_surfaceDestroyed(JNIEnv *env, jobject thiz,
                                                     jlong thread) {
    auto p_thread = (GLThread *) thread;
    jboolean ret = p_thread->surfaceDestroyed(env);
    return ret;
}

JNIEXPORT void JNICALL
Java_com_scut_component_LibGLThread_surfaceUpdated(JNIEnv *env, jobject thiz,
                                                   jlong thread, jobject surface) {
    auto p_thread = (GLThread *) thread;
    p_thread->surfaceUpdated(env, surface);
}

JNIEXPORT void JNICALL
Java_com_scut_component_LibGLThread_destroy(JNIEnv *env, jobject thiz,
                                            jlong thread) {
    auto p_thread = (GLThread *) thread;
    p_thread->waitForExit();
    delete p_thread;
}

// LibSRP.kt
JNIEXPORT jboolean JNICALL
Java_com_scut_utils_LibSRP_create(JNIEnv *env, jobject thiz, jobject controller) {
    if (initialFlag) return true;
    dispatcher = new AudioDataDispatcher();
    audioSource = new AudioRecord(dispatcher, SAMPLE_RATE, FRAME_SIZE);
    snoreJNICallback = new SnoreJNICallback(env, controller);
    splJNICallback = new SPLJNICallback(env, controller);
    snoreThread = new SnoreThread(snoreJNICallback);
    splThread = new SPLThread(splJNICallback);
    dispatcher->registerCallback(splThread);
    dispatcher->registerCallback(snoreThread);
    initialFlag = true;
    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_LibSRP_destroy(JNIEnv *env, jobject thiz, jobject controller) {
    if (!initialFlag) return true;
    audioSource->stop();
    dispatcher->clear();
    snoreThread->waitForExit();
    splThread->waitForExit();
    delete audioSource;
    delete dispatcher;
    delete snoreThread;
    delete splThread;
    snoreJNICallback->updateEnv(env);
    splJNICallback->updateEnv(env);
    delete snoreJNICallback;
    delete splJNICallback;
    initialFlag = false;
    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_LibSRP_start(JNIEnv *env, jobject thiz, jobject controller) {
    if (!initialFlag) return false;
    return audioSource->start();
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_LibSRP_stop(JNIEnv *env, jobject thiz, jobject controller) {
    if (!initialFlag) return false;
    return audioSource->stop();
}

JNIEXPORT jlong JNICALL
Java_com_scut_utils_LibSRP_getSampleRate(JNIEnv *env, jobject thiz, jobject controller) {
    if (!initialFlag) return false;
    return audioSource->getSampleRate();
}

JNIEXPORT jlong JNICALL
Java_com_scut_utils_LibSRP_getStartTime(JNIEnv *env, jobject thiz, jobject controller) {
    if (!initialFlag) return false;
    // TODO delete method or implement
    return 0L;
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_LibSRP_isRunning(JNIEnv *env, jobject thiz, jobject controller) {
    if (!initialFlag) return false;
    return audioSource->isRunning();
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_LibSRP_registerCallback(JNIEnv *env, jobject thiz, jobject controller,
                                            jlong pointer) {
    if (!initialFlag) return false;
    auto callback = (AudioDataCallback *) pointer;
    if (callback == nullptr) return false;
    dispatcher->registerCallback(callback);
    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_scut_utils_LibSRP_unregisterCallback(JNIEnv *env, jobject thiz, jobject controller,
                                              jlong pointer) {
    if (!initialFlag) return false;
    auto callback = (AudioDataCallback *) pointer;
    if (callback == nullptr) return false;
    dispatcher->unregisterCallback(callback);
    return true;
}

JNIEXPORT jlong JNICALL
Java_com_scut_component_RenderFactory_newRender(JNIEnv *env, jobject thiz, jstring type) {
    auto str = env->GetStringUTFChars(type, nullptr);
    GLRender *render = newRender(str);
    env->ReleaseStringUTFChars(type, str);
    return (jlong) render;
}

JNIEXPORT void JNICALL
Java_com_scut_component_RenderFactory_deleteRender(JNIEnv *env, jobject thiz, jlong pointer) {
    auto render = (GLRender *) pointer;
    deleteRender(render);
}
}