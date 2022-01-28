#ifndef SRP_PROJECT_GLTHREAD_H
#define SRP_PROJECT_GLTHREAD_H

#include <jni.h>
#include <thread>
#include "GLRender.h"

using std::thread;
using std::mutex;
using std::atomic;
using std::unique_lock;
using std::condition_variable;

enum class LifecycleState {
    IDLE, CREATE, START, RESUME, PAUSE, STOP, DESTROY
};

class GLThread {
public:
    /**
     * 传递 view 引用和 render
     * @param render
     */
    GLThread(GLRender *render);

    virtual ~GLThread();

    /**
     * 线程运行函数
     * @param env
     */
    void run(JNIEnv *env);

    /**
     * 通知创建 surface 外部接口
     * @param env
     * @param surface
     * @param width
     * @param height
     */
    void surfaceCreate(JNIEnv *env, jobject surface, int32_t width, int32_t height);

    /**
     * 通知大小发生改变外部接口
     * @param env
     * @param surface
     * @param width
     * @param height
     */
    void surfaceSizeChanged(JNIEnv *env, jobject surface, int32_t width, int32_t height);

    /**
     * 通知销毁 surface 外部接口
     * @param env
     * @return
     */
    bool surfaceDestroyed(JNIEnv *env);

    /**
     * 通知更新外部接口，默认没有动作
     * @param env
     * @param surface
     */
    void surfaceUpdated(JNIEnv *env, jobject surface);

    /**
     * 通知生命周期变化
     * @param state
     */
    void onLifecycleChanged(LifecycleState state);

    /**
     * 等待线程退出
     */
    void waitForExit();

private:
    // 线程通信交换变量
    volatile jobject m_surface = nullptr;
    volatile int32_t m_wait = 0;
    volatile int32_t m_width = 0;
    volatile int32_t m_height = 0;
    volatile bool m_exit = false;
    volatile bool m_alive = false;
    volatile bool m_surfaceCreate = false;
    volatile bool m_surfaceDestroy = false;
    volatile bool m_surfaceSizeChange = false;
    volatile LifecycleState m_lifecycle = LifecycleState::IDLE;

    GLRender *m_render;
    mutex m_mutex;
    condition_variable m_cond;
    thread m_thread;
};

#endif