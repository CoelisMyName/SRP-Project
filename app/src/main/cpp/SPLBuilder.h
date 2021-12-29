#ifndef SRP_PROJECT_SPLBUILDER_H
#define SRP_PROJECT_SPLBUILDER_H

#include <jni.h>

/**
 * 辅助构造com.scut.utils.SPL类，能跨线程使用
 */
class SPLBuilder {
public:
    SPLBuilder(JNIEnv *env);

    ~SPLBuilder();

    /**
     * 默认构造函数，返回local引用，需要DeleteLocalRef来回收
     * @param env
     * @return SPL
     */
    jobject getNewSPL(JNIEnv *env);

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
    jobject getNewSPL(JNIEnv *env, jlong timestamp, jdouble a_sum, jdouble c_sum, jdouble z_sum,
                      jdouble freq[8], jdouble a_pow[8], jdouble c_pow[8], jdouble z_pow[8]);

private:
    jclass m_class;///SPL类
    jmethodID m_init, m_init_args;///无参构造函数与有参构造函数
};

#endif