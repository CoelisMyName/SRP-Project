#ifndef SRP_PROJECT_GLRENDER_H
#define SRP_PROJECT_GLRENDER_H

#include <cstdint>

/**
 * OpenGL ES 渲染类接口，传递给 GLThread，由 GLThread 管理 GLRender 接口，以实现绘制
 */
class GLRender {
public:
    virtual ~GLRender() = default;

    /**
     * 添加到 GLThread 后调用，只调用一次，用于做初始化，比如初始化 OpenGL，此时不一定有 EGL 上下文
     */
    virtual void onRenderAttach() = 0;

    /**
     * Surface 初始化
     * EGL 上下文已经初始化完毕
     * @param width 像素值
     * @param height 像素值
     */
    virtual void onSurfaceCreate(int32_t width, int32_t height) = 0;

    /**
     * 绘制函数，调用 OpenGL ES API 进行绘制
     */
    virtual void onSurfaceDraw() = 0;

    /**
     * 视窗大小发生改变
     * @param width 像素值
     * @param height 像素值
     */
    virtual void onSurfaceSizeChange(int32_t width, int32_t height) = 0;

    /**
     * Surface 销毁
     * EGL 上下文即将销毁
     */
    virtual void onSurfaceDestroy() = 0;

    /**
     * 从 GLThread 移除后调用，只调用一次，一般发生在 GLThread 要被回收的时候，做数据销毁，比如销毁 OpenGL
     */
    virtual void onRenderDetach() = 0;
};

#endif