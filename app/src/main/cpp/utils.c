#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <sox.h>
#include <assert.h>
#include <string.h>
#include "utils-sox.h"
#include "utils.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_com_scut_utils_Utils_generateNoiseProfile(JNIEnv *env, jclass clazz,
                                               jbyteArray bytes /* 输入s16le单通道pcm数据 */,
                                               jdouble start /* trim效果器起始位置 */,
                                               jdouble duration /* trim效果器长度 */,
                                               jstring output_file /* noiseprof输出文件路径 */) {
    /** 从Java对象提取数据 */
    void *data = (*env)->GetByteArrayElements(env, bytes, NULL);
    int size = (*env)->GetArrayLength(env, bytes);
    char *p_output_file = (*env)->GetStringUTFChars(env, output_file, NULL);

    generateNoiseProfile(data, size, start, duration, p_output_file);

    /** 释放Java数据 */
    (*env)->ReleaseByteArrayElements(env, bytes, data, JNI_ABORT);
    (*env)->ReleaseStringUTFChars(env, output_file, p_output_file);
}

JNIEXPORT void JNICALL
Java_com_scut_utils_Utils_reduceNoise(JNIEnv *env, jclass clazz, jbyteArray in_bytes,
                                      jbyteArray out_bytes, jstring profile_file,
                                      jdouble parameter) {
}

JNIEXPORT void JNICALL
Java_com_scut_utils_Utils_classifier(JNIEnv *env, jclass clazz, jobject buffer,
                                     jobject classification, jlong minute, jstring profile_file) {
    void *ptr = (*env)->GetDirectBufferAddress(env, buffer);
    const long size = (*env)->GetDirectBufferCapacity(env, buffer);

    void* data = malloc(size);
    memcpy(data, buffer, size);

    const char *profile_file_str = (*env)->GetStringUTFChars(env, profile_file, NULL);

    /** 第0分钟 */
    if(minute == 0) {
        generateNoiseProfile(data, size, 0.0, 1.0, profile_file_str);
    }

    short* output = malloc(size);
    reduceNoise(data, size, output, size, profile_file_str, 0.21);

    const long sample_len = DEFAULT_LENGTH;
    double *sample = malloc(sample_len * sizeof(double));
    for (int i = 0; i < sample_len; ++i) {
        sample[i] = output[i] / 32768.0;
    }

    //TODO vad

    //TODO update profile

    //TODO classifier

    (*env)->ReleaseStringUTFChars(env, profile_file, profile_file_str);
    free(data);
    free(output);
    free(sample);
}

#ifdef __cplusplus
}
#endif

