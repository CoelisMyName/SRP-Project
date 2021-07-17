#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <sox.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_com_scut_utils_Utils_initial(JNIEnv *env, jclass clazz) {
    sox_init();
    sox_format_init();
}

JNIEXPORT void JNICALL
Java_com_scut_utils_Utils_quit(JNIEnv *env, jclass clazz) {
    sox_init();
    sox_format_init();
}

JNIEXPORT void JNICALL
Java_com_scut_utils_Utils_write(JNIEnv *env, jclass clazz, jstring filename, jstring string) {
    const char *p_filename = (*env)->GetStringUTFChars(env, filename, NULL);
    const char *p_string = (*env)->GetStringUTFChars(env, string, NULL);
    int size = (*env)->GetStringUTFLength(env, string);
    FILE *fp = fopen(p_filename, "wb");
    if(fp == NULL) {
        goto finalize;
    }
    fwrite(p_string, 1, size, fp);
    fclose(fp);
    fp = NULL;
    finalize:
    (*env)->ReleaseStringUTFChars(env, filename, p_filename);
    (*env)->ReleaseStringUTFChars(env, string, p_string);
}

#ifdef __cplusplus
}
#endif