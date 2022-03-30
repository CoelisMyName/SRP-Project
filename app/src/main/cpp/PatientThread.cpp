#include "log.h"
#include "utils.h"
#include "PatientThread.h"

TAG(PatientThread)

using std::mutex;
using std::thread;
using std::unique_lock;
using std::condition_variable;
using snore::SNORE_PatientModel;
using snore::deletePatientModel;

PatientThread::PatientThread(PatientJNICallback *callback) : mCallback(callback), mQueue(100),
                                                             mThread([this] {
                                                                 EnvHelper helper;
                                                                 this->run(helper.getEnv());
                                                             }) {
    unique_lock<mutex> lock(mMutex);
    while (!mAlive) {
        mCond.notify_all();
        mCond.wait(lock);
    }
}

void PatientThread::run(JNIEnv *env) {
    unique_lock<mutex> lock(mMutex);
    mAlive = true;
    mCond.notify_all();
    lock.unlock();

    bool exit;
    bool hasTask = false;
    PatientIdentifyTask task;

    while (true) {
        //sync block
        {
            lock.lock();
            while (!mExit && mQueue.empty()) {
                mCond.notify_all();
                mCond.wait(lock);
            }
            exit = mExit;
            if (!exit && !mQueue.empty()) {
                hasTask = true;
                mQueue.poll(task);
            }
            mCond.notify_all();
            lock.unlock();
        }

        if (exit) {
            break;
        }

        if (hasTask) {
            hasTask = false;
            log_i("%s(): get task", __FUNCTION__);
            int64_t timestamp = task.timestamp;
            SNORE_PatientModel *patientModel = task.patientModel;
            if (patientModel != nullptr) {
                double result = task.patientModel->getResult();
                mCallback->onPatientResult(env, timestamp, result);
                deletePatientModel(patientModel);
            } else {
                //TODO recreate patient model
            }
            task = {0L, nullptr};
        }
    }

    lock.lock();
    mAlive = false;
    //discard all task
    while (!mQueue.empty()) {
        mQueue.poll(task);
        //in case memory leak
        deletePatientModel(task.patientModel);
    }
    mCond.notify_all();
    lock.unlock();
}

bool PatientThread::submitTask(int64_t timestamp, snore::SNORE_PatientModel *patientModel) {
    PatientIdentifyTask task = {timestamp, patientModel};
    unique_lock<mutex> lock(mMutex);
    while (!mExit && mAlive) {
        mCond.notify_all();
        if (mQueue.push(task)) return true;
    }
    return false;
}

void PatientThread::waitForExit() {
    unique_lock<mutex> lock(mMutex);
    if (!mAlive) return;
    mExit = true;
    while (mAlive) {
        mCond.notify_all();
        mCond.wait(lock);
    }
    lock.unlock();
    mThread.join();
}
