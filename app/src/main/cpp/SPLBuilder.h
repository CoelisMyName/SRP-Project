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
    SPLBuilder(JNIEnv *env) {
        m_env = env;
        m_class = env->FindClass("com/scut/utils/SPL");
        m_init = env->GetMethodID(m_class, "<init>", "()V");
        m_init_args = env->GetMethodID(m_class, "<init>", "(JDDD[D[D[D[D)V");
    }

    ~SPLBuilder() {
        m_env->DeleteLocalRef(m_class);
    }

    /**
     * 默认构造函数，返回local引用，需要DeleteLocalRef来回收
     * @param env
     * @return SPL
     */
    jobject getNewSPL() {
        return m_env->NewObject(m_class, m_init);
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
    jobject getNewSPL(SPL &spl) {
        jdoubleArray j_freq, j_a_pow, j_c_pow, j_z_pow;
        j_freq = m_env->NewDoubleArray(8);
        j_a_pow = m_env->NewDoubleArray(8);
        j_c_pow = m_env->NewDoubleArray(8);
        j_z_pow = m_env->NewDoubleArray(8);
        m_env->SetDoubleArrayRegion(j_freq, 0, 8, spl.freq);
        m_env->SetDoubleArrayRegion(j_a_pow, 0, 8, spl.a_pow);
        m_env->SetDoubleArrayRegion(j_c_pow, 0, 8, spl.c_pow);
        m_env->SetDoubleArrayRegion(j_z_pow, 0, 8, spl.z_pow);
        jobject obj = m_env->NewObject(m_class, m_init_args, spl.timestamp, spl.a_sum, spl.c_sum,
                                       spl.z_sum, j_freq, j_a_pow, j_c_pow, j_z_pow);
        //释放不被引用的资源
        m_env->DeleteLocalRef(j_freq);
        m_env->DeleteLocalRef(j_a_pow);
        m_env->DeleteLocalRef(j_c_pow);
        m_env->DeleteLocalRef(j_z_pow);
        return obj;
    }

private:
    JNIEnv *m_env;
    jclass m_class; // SPL类
    jmethodID m_init, m_init_args; // 无参构造函数与有参构造函数
};

#endif