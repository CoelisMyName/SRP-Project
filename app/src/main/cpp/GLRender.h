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
     * Surface 初始化
     * @param width 像素值
     * @param height 像素值
     */
    virtual void onCreate(int32_t width, int32_t height) = 0;

    /**
     * 绘制函数，调用 OpenGL ES API 进行绘制
     */
    virtual void onDraw() = 0;

    /**
     * 视窗大小发生改变
     * @param width 像素值
     * @param height 像素值
     */
    virtual void onChange(int32_t width, int32_t height) = 0;

    /**
     * Surface 销毁
     */
    virtual void onDestroy() = 0;
};

#endif