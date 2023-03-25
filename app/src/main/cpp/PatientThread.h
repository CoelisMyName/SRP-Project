#ifndef SRP_PROJECT_PATIENTTHREAD_H
#define SRP_PROJECT_PATIENTTHREAD_H

#include <thread>
#include <snore.h>
#include "AudioDataBuffer.h"
#include "PatientJNICallback.h"

struct PatientIdentifyTask {
public:
    int64_t timestamp;
    snore::SNORE_PatientModel *patientModel;
    PatientIdentifyTask (int64_t _timestamp = 0, snore::SNORE_PatientModel *_patientModel = nullptr):
        timestamp (_timestamp), patientModel (_patientModel) {

    }
    volatile PatientIdentifyTask& operator= (const volatile PatientIdentifyTask &from) volatile {
        this->patientModel = from.patientModel;
        this->timestamp = from.timestamp;
        return *this;
    }

    PatientIdentifyTask& operator= (PatientIdentifyTask &&from) {
        this->patientModel = from.patientModel;
        this->timestamp = from.timestamp;
        return *this;
    }
};

/**
 * PatientModel计算线程，负责计算和回收SNORE_PatientModel
 */
class PatientThread {
public:
    PatientThread(PatientJNICallback *callback);

    void run(JNIEnv *env);

    /**
     * 如果提交失败，需要自己手动回收
     * @param timestamp
     * @param patientModel
     * @return
     */
    bool submitTask(int64_t timestamp, snore::SNORE_PatientModel *patientModel);

    /**
     * 关闭线程，阻塞，直到线程结束
     */
    void waitForExit();

private:
    volatile bool mExit = false;
    volatile bool mAlive = false;
    Queue<PatientIdentifyTask> mQueue;
    PatientJNICallback *mCallback;
    std::mutex mMutex;
    std::condition_variable mCond;
    std::thread mThread;
};

#endif