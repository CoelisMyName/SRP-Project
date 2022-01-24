#ifndef SRP_PROJECT_SPLBUILDER_H
#define SRP_PROJECT_SPLBUILDER_H

#include <jni.h>

typedef struct {
    jlong timestamp;
    jdouble a_sum;
    jdouble c_sum;
    jdouble z_sum;
    jdouble freq[8];
    jdouble a_pow[8];
    jdouble c_pow[8];
    jdouble z_pow[8];
} SPL;

/**
 * 辅助构造com.scut.utils.SPL类，不能跨线程使用
 */
class SPLBuilder {
public:
    /**
     * 在主线程调用，传递主线程 env
     * @param env
     */
    SPLBuilder(JNIEnv *env) {
        m_env = env;
        jclass local_class = env->FindClass("com/scut/utils/SPL");
        m_class = (jclass) env->NewGlobalRef(local_class);
        env->DeleteLocalRef(local_class);
        m_init = env->GetMethodID(m_class, "<init>", "()V");
        m_init_args = env->GetMethodID(m_class, "<init>", "(JDDD[D[D[D[D)V");
    }

    /**
     * 当需要回收对象时更新 env
     * @param env
     */
    void updateEnv(JNIEnv *env) {
        m_env = env;
    }

    ~SPLBuilder() {
        m_env->DeleteGlobalRef(m_class);
    }

    /**
     * 默认构造函数，返回local引用，需要DeleteLocalRef来回收
     * @param env
     * @return SPL
     */
    jobject getNewSPL(JNIEnv *env) {
        return env->NewObject(m_class, m_init);
    }

    /**
     * 有参构造函数，返回local引用，需要DeleteLocalRef来回收
     * @param env
     * @param timestamp
     * @param a_sum
     * @param c_sum
     * @param z_sum
     * @param freq
     * @param a_pow
     * @param c_pow
     * @param z_pow
     * @return SPL
     */
    jobject getNewSPL(JNIEnv *env, SPL &spl) {
        jdoubleArray j_freq, j_a_pow, j_c_pow, j_z_pow;
        j_freq = env->NewDoubleArray(8);
        j_a_pow = env->NewDoubleArray(8);
        j_c_pow = env->NewDoubleArray(8);
        j_z_pow = env->NewDoubleArray(8);
        env->SetDoubleArrayRegion(j_freq, 0, 8, spl.freq);
        env->SetDoubleArrayRegion(j_a_pow, 0, 8, spl.a_pow);
        env->SetDoubleArrayRegion(j_c_pow, 0, 8, spl.c_pow);
        env->SetDoubleArrayRegion(j_z_pow, 0, 8, spl.z_pow);
        jobject obj = env->NewObject(m_class, m_init_args, spl.timestamp, spl.a_sum, spl.c_sum,
                                     spl.z_sum, j_freq, j_a_pow, j_c_pow, j_z_pow);
        //释放不被引用的资源
        env->DeleteLocalRef(j_freq);
        env->DeleteLocalRef(j_a_pow);
        env->DeleteLocalRef(j_c_pow);
        env->DeleteLocalRef(j_z_pow);
        return obj;
    }

private:
    JNIEnv *m_env;
    jclass m_class; // SPL类
    jmethodID m_init, m_init_args; // 无参构造函数与有参构造函数
};

#endif