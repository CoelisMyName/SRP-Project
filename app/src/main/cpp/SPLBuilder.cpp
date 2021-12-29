#include "utils.h"
#include "SPLBuilder.h"

SPLBuilder::SPLBuilder(JNIEnv *env) {
    jclass jc_tmp = env->FindClass("com/scut/utils/SPL");
    m_class = (jclass) env->NewGlobalRef(jc_tmp);
    env->DeleteLocalRef(jc_tmp);
    m_init = env->GetMethodID(m_class, "<init>", "()V");
    m_init_args = env->GetMethodID(m_class, "<init>", "(JDDD[D[D[D[D)V");
}

SPLBuilder::~SPLBuilder() {
    EnvHelper helper;
    JNIEnv *env = helper.getEnv();
    env->DeleteLocalRef(m_class);
}

jobject SPLBuilder::getNewSPL(JNIEnv *env) {
    return env->NewObject(m_class, m_init);
}

jobject
SPLBuilder::getNewSPL(JNIEnv *env, jlong timestamp, jdouble a_sum, jdouble c_sum, jdouble z_sum,
                      jdouble freq[8],
                      jdouble a_pow[8], jdouble c_pow[8], jdouble z_pow[8]) {
    jdoubleArray j_freq, j_a_pow, j_c_pow, j_z_pow;
    j_freq = env->NewDoubleArray(8);
    j_a_pow = env->NewDoubleArray(8);
    j_c_pow = env->NewDoubleArray(8);
    j_z_pow = env->NewDoubleArray(8);
    env->SetDoubleArrayRegion(j_freq, 0, 8, freq);
    env->SetDoubleArrayRegion(j_a_pow, 0, 8, a_pow);
    env->SetDoubleArrayRegion(j_c_pow, 0, 8, c_pow);
    env->SetDoubleArrayRegion(j_z_pow, 0, 8, z_pow);
    jobject spl = env->NewObject(m_class, m_init_args, timestamp, a_sum, c_sum, z_sum, j_freq,
                                 j_a_pow,
                                 j_c_pow, j_z_pow);
    //释放不被引用的资源
    env->DeleteLocalRef(j_freq);
    env->DeleteLocalRef(j_a_pow);
    env->DeleteLocalRef(j_c_pow);
    env->DeleteLocalRef(j_z_pow);
    return spl;
}