#include <cassert>
#include <EGL/egl.h>
#include <android/surface_texture.h>
#include <android/surface_texture_jni.h>
#include "log.h"
#include "utils.h"
#include "global.h"
#include "GLThread.h"

using std::thread;
using std::mutex;
using std::atomic;
using std::unique_lock;
using std::condition_variable;

TAG(GLThread)

static const EGLint DEFAULT_RENDER = EGL_OPENGL_ES3_BIT;
#define DEFAULT_SIZE(n, s) static const EGLint DEFAULT_##n##_SIZE = s; // 默认变量值宏定义
// 以下定义 EGL 图形缓冲区值大小
DEFAULT_SIZE(RED, 8)
DEFAULT_SIZE(GREEN, 8)
DEFAULT_SIZE(BLUE, 8)
DEFAULT_SIZE(ALPHA, 8)
DEFAULT_SIZE(DEPTH, 16)
DEFAULT_SIZE(STENCIL, 0)

// EGL 配置串
static EGLint CONFIG_ATTR[] = {EGL_RED_SIZE, DEFAULT_RED_SIZE, EGL_GREEN_SIZE, DEFAULT_GREEN_SIZE,
                               EGL_BLUE_SIZE, DEFAULT_BLUE_SIZE, EGL_ALPHA_SIZE, DEFAULT_ALPHA_SIZE,
                               EGL_DEPTH_SIZE, DEFAULT_ALPHA_SIZE, EGL_STENCIL_SIZE,
                               DEFAULT_STENCIL_SIZE,
                               EGL_RENDERABLE_TYPE, DEFAULT_RENDER, EGL_NONE};

static EGLint CONTEXT_ATTR[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};

// EGL 结构体
typedef struct {
    EGLDisplay display;
    EGLint major, minor;
    EGLConfig config;
    EGLContext context;
    EGLSurface surface;
    EGLint error;
    ANativeWindow *window;
    ASurfaceTexture *surfaceTexture;
} EGLObject;

static bool initialEGL(EGLObject &eglObject); // 初始化 EGL

static bool createEGLSurface(EGLObject &eglObject, JNIEnv *env, jobject surface); // 初始化绘制 Surface

static bool destroyEGLSurface(EGLObject &eglObject); // 销毁绘制 Surface

static bool destroyEGL(EGLObject &eglObject); // 销毁 EGL

static const char *getRenderTypeString(EGLint renderType); // 返回渲染器字符串

__unused static void logConfig(EGLDisplay display, EGLConfig config); // 调试打印 EGL 配置信息

static const char *getErrorMessage(EGLint error); // 返回 EGL 错误字符串

static bool lifecycleVisible(LifecycleState state);

static bool lifecycleInvisible(LifecycleState state);

static const char *lifecycleString(LifecycleState state);

GLThread::GLThread(GLRender *render) : mRender(render),
                                       mThread([this] {
                                           EnvHelper helper;
                                           this->run(helper.getEnv());
                                       }) {
    unique_lock<mutex> lock(mMutex);
    while (!mAlive) {
        mCond.wait(lock);
    }
}

GLThread::~GLThread() {
    if (mAlive) waitForExit();
}

void GLThread::run(JNIEnv *env) {
    unique_lock<mutex> lock(mMutex);
    mAlive = true;
    mCond.notify_all();
    lock.unlock();

    jobject surface = nullptr;
    bool hasSurface = false, exit = false, createSurface = false, destroySurface = false, sizeChange = false;
    int32_t width = 0, height = 0;
    LifecycleState lifecycle = LifecycleState::IDLE;

    mRender->onRenderAttach();
    EGLObject eglObject;
    while (!initialEGL(eglObject)) {
        log_e("%s(): egl initial error %s", __FUNCTION__, getErrorMessage(eglObject.error));
    }

    while (true) {
        // 同步状态
        if (mWait > 0 || !hasSurface || lifecycleInvisible(lifecycle)) {
            lock.lock();
            // 等待条件，有请求，没有 surface，没有创建 surface 的请求，没有退出请求，生命周期没变化
            while (true) {
                if (mExit) break;
                if (mWait > 0) break;
                if (mLifecycle != LifecycleState::IDLE) break;
                if (mSurfaceCreate) break;
                if (mSurfaceDestroy) break;
                mCond.notify_all();
                mCond.wait(lock);
            }
            log_i("%s(): wait = %d", __FUNCTION__, mWait);
            exit = mExit;
            if (mSurfaceCreate) {
                mSurfaceCreate = false;
                width = mWidth;
                height = mHeight;
                createSurface = true;
                surface = env->NewLocalRef(mSurface);
            }
            if (mSurfaceSizeChange) {
                mSurfaceSizeChange = false;
                width = mWidth;
                height = mHeight;
                sizeChange = true;
            }
            if (mSurfaceDestroy) {
                mSurfaceDestroy = false;
                width = 0;
                height = 0;
                destroySurface = true;
            }
            if (mLifecycle != LifecycleState::IDLE) {
                lifecycle = mLifecycle;
                mLifecycle = LifecycleState::IDLE;
                log_i("%s(): lifecycle change %s", __FUNCTION__, lifecycleString(lifecycle));
            }
            mWait = 0;
            mCond.notify_all();
            lock.unlock();
        }
        // 开始工作
        if (exit) {
            if (hasSurface) {
                mRender->onSurfaceDestroy();
            }
            break;
        }
        // EGL API 可能线程不安全
        // 创建 surface
        if (createSurface) {
            createSurface = false;
            while (!createEGLSurface(eglObject, env, surface)) {
                log_e("%s(): egl create surface error %s", __FUNCTION__,
                      getErrorMessage(eglObject.error));
            }
            eglMakeCurrent(eglObject.display, eglObject.surface, eglObject.surface,
                           eglObject.context);
            hasSurface = true;
            env->DeleteLocalRef(surface);
            surface = nullptr;
            mRender->onSurfaceCreate(width, height);
            log_i("%s(): surface create", __FUNCTION__);
        }
        // EGL API 可能线程不安全
        // 销毁 surface
        if (destroySurface) {
            destroySurface = false;
            mRender->onSurfaceDestroy();
            if (!destroyEGLSurface(eglObject)) {
                log_e("%s(): egl destroy surface error %s", __FUNCTION__,
                      getErrorMessage(eglObject.error));
            }
            hasSurface = false;
            log_i("%s(): surface destroy", __FUNCTION__);
        }
        // 大小发生改变
        if (sizeChange) {
            sizeChange = false;
            mRender->onSurfaceSizeChange(width, height);
            log_i("%s(): surface size change width = %d, height = %d", __FUNCTION__, width, height);
        }
        // 渲染 当生命周期在 START 以前，PAUSE 以后就不绘制
        if (hasSurface && lifecycleVisible(lifecycle)) {
            eglMakeCurrent(eglObject.display, eglObject.surface, eglObject.surface,
                           eglObject.context);
            mRender->onSurfaceDraw();
//            log_i("%s(): swap buffer", __FUNCTION__);
            EGLBoolean ret = eglSwapBuffers(eglObject.display, eglObject.surface);
            if (ret == EGL_FALSE) {
                log_e("%s(): swap buffer error %s", __FUNCTION__, getErrorMessage(eglGetError()));
            }
            if (ret == EGL_BAD_SURFACE) {
                destroyEGLSurface(eglObject);
                hasSurface = false;
            }
        }
    }

    mRender->onRenderDetach();
    lock.lock();
    destroyEGLSurface(eglObject);
    destroyEGL(eglObject);
    mAlive = false;
    mCond.notify_all();
    log_i("%s(): exiting", __FUNCTION__);
    lock.unlock();
}

void GLThread::surfaceCreate(JNIEnv *env, jobject surface, int32_t width, int32_t height) {
    jobject g_surface = env->NewGlobalRef(surface);
    unique_lock<mutex> lock(mMutex);
    mWidth = width;
    mHeight = height;
    mSurfaceCreate = true;
    mSurface = g_surface;
    while (!mExit && mAlive && mSurfaceCreate) {
        mWait += 1;
        mCond.notify_all();
        mCond.wait(lock);
    }
    lock.unlock();
    env->DeleteGlobalRef(g_surface);
}

void GLThread::surfaceSizeChanged(JNIEnv *env, jobject surface, int32_t width, int32_t height) {
    unique_lock<mutex> lock(mMutex);
    mSurfaceSizeChange = true;
    mWidth = width;
    mHeight = height;
    while (!mExit && mAlive && mSurfaceSizeChange) {
        mWait += 1;
        mCond.notify_all();
        mCond.wait(lock);
    }
    lock.unlock();
}

bool GLThread::surfaceDestroyed(JNIEnv *env) {
    unique_lock<mutex> lock(mMutex);
    mSurfaceDestroy = true;
    mWait += 1;
//    while (!mExit && mAlive && mSurfaceDestroy) {
//        mWait += 1;
//        mCond.notify_all();
//        mCond.wait(lock);
//    }
    lock.unlock();
    return true;
}

void GLThread::surfaceUpdated(JNIEnv *env, jobject surface) {
}

void GLThread::waitForExit() {
    unique_lock<mutex> lock(mMutex);
    if (!mAlive) return;
    mExit = true;
    while (mAlive) {
        mWait += 1;
        mCond.notify_all();
        mCond.wait(lock);
    }
    lock.unlock();
    mThread.join();
}

void GLThread::lifecycleChanged(LifecycleState state) {
    unique_lock<mutex> lock(mMutex);
    mLifecycle = state;
    mWait += 1;
    mCond.notify_all();
    // 如果主线程被阻塞，Render 调用 glClear 的时候会阻塞，原因可能是因为主线程需要处理
    lock.unlock();
}

static bool initialEGL(EGLObject &eglObject) {
    eglObject = {EGL_NO_DISPLAY, 0, 0, nullptr, EGL_NO_CONTEXT, EGL_NO_SURFACE, EGL_SUCCESS,
                 nullptr,
                 nullptr};
    EGLBoolean ret = EGL_FALSE;
    EGLDisplay dpy = EGL_NO_DISPLAY;
    EGLint mjr = 0, mnr = 0, num = 0, err = EGL_SUCCESS;
    EGLConfig cfg = nullptr, *cfgs = nullptr;
    EGLContext ctx = EGL_NO_CONTEXT;

    dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (dpy == EGL_NO_DISPLAY) goto finish;

    ret = eglInitialize(dpy, &mjr, &mnr);
    if (ret == EGL_FALSE) goto finish;

    ret = eglChooseConfig(dpy, CONFIG_ATTR, nullptr, 0, &num);
    assert(num > 0);
    if (ret == EGL_FALSE) goto finish;
    log_i("%s(): number of config %d", __FUNCTION__, num);

    cfgs = new EGLConfig[num];
    ret = eglChooseConfig(dpy, CONFIG_ATTR, cfgs, num, &num);
    if (ret == EGL_FALSE) goto finish;
//    for (uint32_t i = 0; i < num; ++i) logConfig(dpy, cfgs[i]);

    cfg = cfgs[0];
    ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, CONTEXT_ATTR);
    if (ctx == EGL_NO_CONTEXT) goto finish;

    finish:
    delete[] cfgs;
    err = eglGetError();
    eglObject = {dpy, mjr, mnr, cfg, ctx, nullptr, err};
    return err == EGL_SUCCESS;
}

static bool createEGLSurface(EGLObject &eglObject, JNIEnv *env, jobject surface) {
    EGLSurface sfc = EGL_NO_SURFACE;
    EGLint err = EGL_SUCCESS;
    eglObject.surfaceTexture = ASurfaceTexture_fromSurfaceTexture(env, surface);
    eglObject.window = ASurfaceTexture_acquireANativeWindow(eglObject.surfaceTexture);
    sfc = eglCreateWindowSurface(eglObject.display, eglObject.config, eglObject.window, nullptr);
    err = eglGetError();
    eglObject.surface = sfc;
    eglObject.error = err;
    return sfc != EGL_NO_SURFACE;
}

static bool destroyEGLSurface(EGLObject &eglObject) {
    EGLBoolean ret;
    EGLint err;
    ret = eglMakeCurrent(eglObject.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//    if (ret == EGL_FALSE) goto finish;
    if (eglObject.surface != EGL_NO_SURFACE) {
        ret = eglDestroySurface(eglObject.display, eglObject.surface);
    }
//    if (ret == EGL_FALSE) goto finish;
    eglObject.surface = EGL_NO_SURFACE;
    if (eglObject.window != nullptr) {
        ANativeWindow_release(eglObject.window);
    }
    eglObject.window = nullptr;
    if (eglObject.surfaceTexture != nullptr) {
        ASurfaceTexture_release(eglObject.surfaceTexture);
    }
    eglObject.surfaceTexture = nullptr;
//    finish:
    err = eglGetError();
    eglObject.error = err;
    return err == EGL_SUCCESS;
}

static bool destroyEGL(EGLObject &eglObject) {
    EGLBoolean ret;
    ret = eglDestroyContext(eglObject.display, eglObject.context);
    if (ret != EGL_TRUE) goto finish;
    eglObject.context = EGL_NO_CONTEXT;

    finish:
    eglObject.error = eglGetError();
    return ret == EGL_TRUE;
}

static const char *getRenderTypeString(EGLint renderType) {
    if (renderType & EGL_OPENGL_ES3_BIT) {
        return "opengl es 3";
    }
    if (renderType & EGL_OPENGL_ES2_BIT) {
        return "opengl es 2";
    }
    if (renderType & EGL_OPENGL_BIT) {
        return "opengl es 1";
    }
    if (renderType & EGL_OPENVG_BIT) {
        return "openvg";
    }
    return "unknown";
}

__unused static void logConfig(EGLDisplay display, EGLConfig config) {
    EGLint redSize, greenSize, blueSize, alphaSize, depthSize, stencilSize, renderType;
    eglGetConfigAttrib(display, config, EGL_RED_SIZE, &redSize);
    eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &greenSize);
    eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blueSize);
    eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &alphaSize);
    eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depthSize);
    eglGetConfigAttrib(display, config, EGL_STENCIL_SIZE, &stencilSize);
    eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &renderType);
    log_i("%s(): red size = %d, green size = %d, blue size = %d, alpha size = %d, depth size = %d, stencil size = %d, render type = %s",
          __FUNCTION__, redSize, greenSize, blueSize, alphaSize, depthSize, stencilSize,
          getRenderTypeString(renderType));
}

static const char *getErrorMessage(EGLint error) {
    switch (error) {
        case EGL_SUCCESS:
            return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED:
            return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS:
            return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC:
            return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE:
            return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONFIG:
            return "EGL_BAD_CONFIG";
        case EGL_BAD_CONTEXT:
            return "EGL_BAD_CONTEXT";
        case EGL_BAD_CURRENT_SURFACE:
            return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY:
            return "EGL_BAD_DISPLAY";
        case EGL_BAD_MATCH:
            return "EGL_BAD_MATCH";
        case EGL_BAD_NATIVE_PIXMAP:
            return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW:
            return "EGL_BAD_NATIVE_WINDOW";
        case EGL_BAD_PARAMETER:
            return "EGL_BAD_PARAMETER";
        case EGL_BAD_SURFACE:
            return "EGL_BAD_SURFACE";
        case EGL_CONTEXT_LOST:
            return "EGL_CONTEXT_LOST";
        default:
            return "UNKNOWN EGL ERROR";
    }
}

static bool lifecycleVisible(LifecycleState state) {
    return state == LifecycleState::START || state == LifecycleState::RESUME;
}

static bool lifecycleInvisible(LifecycleState state) {
    return state == LifecycleState::IDLE || state == LifecycleState::CREATE ||
           state == LifecycleState::PAUSE || state == LifecycleState::STOP ||
           state == LifecycleState::DESTROY;
}

static const char *lifecycleString(LifecycleState state) {
    switch (state) {
        case LifecycleState::IDLE:
            return "IDLE";
        case LifecycleState::CREATE:
            return "CREATE";
        case LifecycleState::START:
            return "START";
        case LifecycleState::RESUME:
            return "RESUME";
        case LifecycleState::PAUSE:
            return "PAUSE";
        case LifecycleState::STOP:
            return "STOP";
        case LifecycleState::DESTROY:
            return "DESTROY";
    }
    return "";
}