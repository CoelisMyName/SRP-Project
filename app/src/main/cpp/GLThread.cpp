#include <cassert>
#include <EGL/egl.h>
#include <android/surface_texture.h>
#include <android/surface_texture_jni.h>
#include "log.h"
#include "global.h"
#include "GLThread.h"

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
} EGLObject;

static bool initialEGL(EGLObject &eglObject); // 初始化 EGL

static bool createEGLSurface(EGLObject &eglObject, JNIEnv *env, jobject surface); // 初始化绘制 Surface

static bool destroyEGLSurface(EGLObject &eglObject); // 销毁绘制 Surface

static bool destroyEGL(EGLObject &eglObject); // 销毁 EGL

static const char *getRenderTypeString(EGLint renderType); // 返回渲染器字符串

static void logConfig(EGLDisplay display, EGLConfig config); // 调试打印 EGL 配置信息

static void attachGLThread(GLThread *glThread); // 线程运行 GLThread 调用函数

static const char *getErrorMessage(EGLint error); // 返回 EGL 错误字符串

GLThread::GLThread(JNIEnv *env, jobject view, GLRender *render) : m_render(render), m_width(0),
                                                                  m_height(0),
                                                                  m_alive(false), m_exit(false),
                                                                  m_wait(0),
                                                                  m_surfaceSizeChange(false),
                                                                  m_surfaceCreate(false),
                                                                  m_surfaceDestroy(false),
                                                                  m_surface(nullptr),
                                                                  m_thread([this] {
                                                                      attachGLThread(this);
                                                                  }) {
    unique_lock<mutex> lock(m_mutex);
    jweak w_view = env->NewWeakGlobalRef(view);
    m_view = w_view;
    while (!m_alive) {
        m_cond.wait(lock);
    }
}

GLThread::~GLThread() {
    if (m_alive) waitForExit();
}

void GLThread::run(JNIEnv *env) {
    unique_lock<mutex> lock(m_mutex);
    m_alive = true;
    m_cond.notify_all();
    log_i("%s(): is alive", __FUNCTION__);
    lock.unlock();

    jobject surface = nullptr;
    bool hasSurface = false, exit = false, createSurface = false, destroySurface = false, sizeChange = false;
    int32_t width = 0, height = 0;

    EGLObject eglObject;
    while (!initialEGL(eglObject)) {
        log_e("%s(): egl initial error %s", __FUNCTION__, getErrorMessage(eglObject.error));
    }

    while (true) {
        // 同步状态
        if (m_wait > 0 || !hasSurface) {
            lock.lock();
            log_i("%s(): wait = %d", __FUNCTION__, m_wait);
            // 等待条件，有请求，没有 surface，没有创建 surface 的请求，没有退出请求
            while (m_wait <= 0 && (!hasSurface && !m_surfaceCreate) && !m_exit) {
                m_cond.notify_all();
                m_cond.wait(lock);
            }
            exit = m_exit;
            if (m_surfaceCreate) {
                m_surfaceCreate = false;
                width = m_width;
                height = m_height;
                createSurface = true;
                surface = env->NewLocalRef(m_surface);
            }

            if (m_surfaceSizeChange) {
                m_surfaceSizeChange = false;
                width = m_width;
                height = m_height;
                sizeChange = true;
            }

            if (m_surfaceDestroy) {
                m_surfaceDestroy = false;
                width = 0;
                height = 0;
                destroySurface = true;
            }

            m_wait = 0;
            m_cond.notify_all();
            lock.unlock();
        }
        // 开始工作
        if (exit) {
            if (hasSurface) {
                m_render->onDestroy();
            }
            hasSurface = false;
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
            m_render->onCreate(width, height);
            log_i("%s(): surface create", __FUNCTION__);
        }
        // EGL API 可能线程不安全
        // 销毁 surface
        if (destroySurface) {
            destroySurface = false;
            m_render->onDestroy();
            destroyEGLSurface(eglObject);
            hasSurface = false;
            log_i("%s(): surface destroy", __FUNCTION__);
        }

        // 大小发生改变
        if (sizeChange) {
            sizeChange = false;
            m_render->onChange(width, height);
            log_i("%s(): surface size change width = %d, height = %d", __FUNCTION__, width, height);
        }

        // 渲染
        if (hasSurface) {
            m_render->onDraw();
            EGLBoolean ret = eglSwapBuffers(eglObject.display, eglObject.surface);
            log_i("%s(): drawing", __FUNCTION__);
            if (ret == EGL_FALSE) {
                log_e("%s(): swap buffer error %s", __FUNCTION__, getErrorMessage(eglGetError()));
            }
        }
    }

    lock.lock();
    destroyEGLSurface(eglObject);
    destroyEGL(eglObject);
    env->DeleteWeakGlobalRef(m_view);
    m_alive = false;
    m_cond.notify_all();
    log_i("%s(): exiting", __FUNCTION__);
    lock.unlock();
}

void GLThread::surfaceCreate(JNIEnv *env, jobject surface, int32_t width, int32_t height) {
    jobject g_surface = env->NewGlobalRef(surface);
    unique_lock<mutex> lock(m_mutex);
    m_width = width;
    m_height = height;
    m_surfaceCreate = true;
    m_surface = g_surface;
    while (!m_exit && m_alive && m_surfaceCreate) {
        m_wait += 1;
        m_cond.notify_all();
        m_cond.wait(lock);
    }
    lock.unlock();
    env->DeleteGlobalRef(g_surface);
}

void GLThread::surfaceSizeChanged(JNIEnv *env, jobject surface, int32_t width, int32_t height) {
    unique_lock<mutex> lock(m_mutex);
    m_surfaceSizeChange = true;
    m_width = width;
    m_height = height;
    while (!m_exit && m_alive && m_surfaceSizeChange) {
        m_wait += 1;
        m_cond.notify_all();
        m_cond.wait(lock);
    }
    lock.unlock();
}

bool GLThread::surfaceDestroyed(JNIEnv *env) {
    unique_lock<mutex> lock(m_mutex);
    m_surfaceDestroy = true;
    while (!m_exit && m_alive && m_surfaceDestroy) {
        m_wait += 1;
        m_cond.notify_all();
        m_cond.wait(lock);
    }
    lock.unlock();
    return true;
}

void GLThread::surfaceUpdated(JNIEnv *env, jobject surface) {
}

void GLThread::waitForExit() {
    unique_lock<mutex> lock(m_mutex);
    m_exit = true;
    while (m_alive) {
        m_wait += 1;
        m_cond.notify_all();
        m_cond.wait(lock);
    }
    lock.unlock();
    m_thread.join();
}

static bool initialEGL(EGLObject &eglObject) {
    eglObject = {EGL_NO_DISPLAY, 0, 0, nullptr, EGL_NO_CONTEXT, EGL_NO_SURFACE, EGL_SUCCESS};
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
    for (uint32_t i = 0; i < num; ++i) logConfig(dpy, cfgs[i]);

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
    ASurfaceTexture *aSurfaceTexture = ASurfaceTexture_fromSurfaceTexture(env, surface);
    ANativeWindow *win = ASurfaceTexture_acquireANativeWindow(aSurfaceTexture);
    sfc = eglCreateWindowSurface(eglObject.display, eglObject.config, win, nullptr);
    ANativeWindow_release(win);
    ASurfaceTexture_release(aSurfaceTexture);
    err = eglGetError();
    eglObject.surface = sfc;
    eglObject.error = err;
    return sfc != EGL_NO_SURFACE;
}

static bool destroyEGLSurface(EGLObject &eglObject) {
    EGLBoolean ret;
    EGLint err;
    ret = eglMakeCurrent(eglObject.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (ret == EGL_FALSE) goto finish;
    ret = eglDestroySurface(eglObject.display, eglObject.surface);
    if (ret == EGL_FALSE) goto finish;
    eglObject.surface = EGL_NO_SURFACE;

    finish:
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

static void logConfig(EGLDisplay display, EGLConfig config) {
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

static void attachGLThread(GLThread *glThread) {
    JNIEnv *env = nullptr;
    jint res;
    res = g_jvm->AttachCurrentThread(&env, nullptr);
    assert(res == JNI_OK);
    glThread->run(env);
    res = g_jvm->DetachCurrentThread();
    assert(res == JNI_OK);
    env = nullptr;
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