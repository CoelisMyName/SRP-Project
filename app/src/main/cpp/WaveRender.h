#ifndef SRP_PROJECT_WAVERENDER_H
#define SRP_PROJECT_WAVERENDER_H

#if __ANDROID_API__ >= 24

#include <GLES3/gl32.h>

#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#include "GLRender.h"
#include <thread>
#include "AudioGLRender.h"
#include "AudioDataBuffer.h"

#define WAVERENDER_SYNC_RISK true

#ifndef WAVERENDER_SYNC_RISK
#include <mutex>
#endif // ! WAVERENDER_SYNC_RISK

// 接收缓冲区大小，应当大于 WAVERENDER_MAX_WIDTH
#define WAVERENDER_BUFFER_SIZE 8192
// 实际可被显示的波形数组最大大小，也是显示缓冲区的大小
#define WAVERENDER_MAX_WIDTH 7500

#if WAVERENDER_BUFFER_SIZE <= WAVERENDER_MAX_WIDTH
#error MACRO [WAVERENDER_BUFFER_SIZE] should greater than MACRO [WAVERENDER_MAX_WIDTH]
#endif // 检查上面两个参数的配置是否正确

// _render_convert() 会对视图内的波形进行缩放，视图的Y轴范围是动态的 [-y,y] 区间.
// y 的最大取值为 INT16_MAX
// 该宏控制y的最小取值
#define WAVERENDER_SCALE_PROTECTED_HEIGHT 8191
// 该宏控制 y 在相邻两帧内的最大变化量
#define WAVERENDER_SCALE_MAX_STEP 600

#define WAVERENDER_COLOR2F(XX) (((float)(0x##XX))/255.0f)

#define WAVERENDER_COLOR(A,R,G,B)             {\
        WAVERENDER_COLOR2F(R),                 \
        WAVERENDER_COLOR2F(G),                 \
        WAVERENDER_COLOR2F(B),                 \
        WAVERENDER_COLOR2F(A)}

// 这四个宏控制 亮色/暗色模式 下绘制区域的背景颜色，顶点颜色在着色器改
#define WAVERENDER_COLOR_LIGHT_BACKGROUND           WAVERENDER_COLOR(FF,E0,E0,E7)
#define WAVERENDER_COLOR_DARK_BACKGROUND            WAVERENDER_COLOR(FF,20,20,20)

class WaveRender : public AudioGLRender {
public:
    WaveRender();

    virtual ~WaveRender() override;

    virtual void onAudioCallbackAttach() override;

    virtual void onAudioDataStart(int64_t timestamp) override;

    virtual void onAudioDataStop(int64_t timestamp) override;

    virtual void onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) override;

    virtual void onAudioCallbackDetach() override;

    virtual void onRenderAttach() override;

    virtual void onSurfaceCreate(int32_t width, int32_t height) override;

    virtual void onSurfaceDraw() override;

    virtual void onSurfaceSizeChange(int32_t width, int32_t height) override;

    virtual void onSurfaceDestroy() override;

    virtual void onRenderDetach() override;

    virtual void onDarkModeChange(bool intoDarkMode) override;

private:
    int32_t mWidth = 0, mHeight = 0;
    bool mInit = false;
    GLuint mPgrLight = 0,mPgrDark=0;
    GLuint mVbo = 0, mVao = 0;

#ifndef WAVERENDER_SYNC_RISK
    std::mutex mMutex;
#endif // ! WAVERENDER_SYNC_RISK

    /**
     * @brief 波形条结构，仅在接收与处理阶段使用
     *
     */
    struct _wave_t {
        int16_t minimun;
        int16_t maximun;
    };
    /**
     * @brief 接收缓冲区使用的公共内存空间。
     * 除初始化及特殊说明外，本区域变量遵循receive读写、draw只读的约定
     */
    struct _recv_data_t {
        /* 缓冲区控制 开始 */

        // 接收主缓存，数组型循环列表。存放接收到的 min - max 对，配合start_pos使用
        _wave_t buffer[WAVERENDER_BUFFER_SIZE];
        // 当前可用的 min - max 对数量。仅在对象创建早期、显示区域宽高变化后的短时间内会改变
        // 该值取值范围为 [0, WAVERENDER_MAX_WIDTH]
        volatile size_t available_length;
        // 循环列表当前表头位置由draw()负责，通过 next_pos 计算得出
        // 循环列表下一次插入点位置（仅receive使用）
        volatile size_t next_pos;

        /* 缓冲区控制 结束 */

        /* 半组数据使用区 开始 */
        /* 本组变量用于跨receive提取最值，仅receive使用 */

        // 当前波形条内已读到的最大值
        int16_t cur_max;
        // 当前波形条内已读到的最小值
        int16_t cur_min;
        // 当前波形条中已过滤的长度，用于在相邻receieve过程间提取峰值
        uint32_t cur_pos;

        /* 半组数据使用区 结束 */

    } _m_recvDatas;

    /**
     * @brief 显示缓冲区相关参数。每次onSurfaceDraw的调用都会覆盖其中的内容
     * 除初始化外，本区域仅供draw读写
     */
    struct _rend_data_t {
        /* 缓冲区控制 开始 */

        // 渲染用缓冲区。拷贝并整理自 this->_m_recvDatas.buffer .
        // buffer[0]即为首个可用数据
        _wave_t buffer[WAVERENDER_MAX_WIDTH];
        // 缓冲区实际绘制大小。拷贝自 this->_m_recvDatas.available_length
        size_t size;

        /* 缓冲区控制 结束 */
        /* 波形幅值缩放控制区 开始 */

        // 上次绘制中的最值，用于平滑更改缩放系数
        // 该值的取值为 min的绝对值与max的绝对值中较大的一个
        // 该值最大取值为 INT16_MAX
        // 除初始化外，该值只被 _render_convert() 访问
        int16_t last_minmax;

        /* 波形幅值缩放控制区 结束 */

        // 绘图缓冲区. 
        // 该缓冲区无需初始化也可正常运行
        // 第 [6*x: 6*(x+1)] 区域的元素依次为： x1,y1,0,x2,y2,0
        GLfloat output_buffer[WAVERENDER_MAX_WIDTH * 6];

    } _m_renderDatas;

    /* 缓冲区初始化函数 开始*/
    // 清理接收缓冲区
    void _recv_dataReinit();

    // 清理绘制缓冲区
    void _render_dataReinit();
    /* 缓冲区初始化函数 结束*/

    // 复制缓冲区内容
    void _copy_from_receiver();

    // 映射渲染缓冲内的波形高度
    void _render_convert();

    // 从波形高度生成坐标
    void _render_genPoints();

    bool _m_isAtDarkMode = false;
};

#endif