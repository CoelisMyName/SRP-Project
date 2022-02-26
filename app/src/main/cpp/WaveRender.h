#ifndef SRP_PROJECT_WAVERENDER_H
#define SRP_PROJECT_WAVERENDER_H

#if __ANDROID_API__ >= 24

#include <GLES3/gl32.h>

#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#else
#include <GLES3/gl3.h>
#endif

#include <thread>
#include "GLRender.h"
#include "AudioDataBuffer.h"
#include "AudioDataCallback.h"

#define WAVERENDER_SYNC_RISK true

#ifndef WAVERENDER_SYNC_RISK
#include <mutex>
#endif  // ! WAVERENDER_SYNC_RISK

// 接收缓冲区大小，应当大于 WAVERENDER_MAX_WIDTH
#define WAVERENDER_BUFFER_SIZE 8192
// 实际可被显示的波形数组最大大小，也是显示缓冲区的大小
#define WAVERENDER_MAX_WIDTH 7500

class WaveRender : public GLRender, public AudioDataCallback {
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

private:
    int32_t mWidth = 0, mHeight = 0;
    bool mInit = false;
    GLuint mPgr = 0;
    GLuint mVbo = 0, mVao = 0;

    std::mutex mMutex;

    int32_t mAudioBufferCapacity;
    int32_t mAudioBufferSize = 0;
    int16_t *mAudioBuffer;

    Queue<int16_t> mMaxQueue;
    Queue<int16_t> mMinQueue;

    int32_t mBufferCapacity;
    int32_t mBufferSize = 0;
    int16_t *mMaxBuffer;
    int16_t *mMinBuffer;
    

    /**
     * @brief 波形条结构，仅在接收与处理阶段使用
     * 
     */
    struct _wave_t{
        int16_t minimun;
        int16_t maximun;
    };
    /**
     * @brief 接收缓冲区使用的公共内存空间。
     * 除初始化及特殊说明外，本区域变量遵循receive读写、draw只读的约定
     */
    struct _recv_data_t
    {
        /* 缓冲区控制 开始 */

        // 接收主缓存，数组型循环列表。存放接收到的 min - max 对，配合start_pos使用
        _wave_t buffer[WAVERENDER_BUFFER_SIZE];
        // 当前可用的 min - max 对数量。仅在对象创建早期、显示区域宽高变化后的短时间内会改变
        // 该值取值范围为 [0, x] , 其中 x 为 this->mWidth 与 WAVERENDER_MAX_WIDTH 中的较小值
        volatile size_t available_length;
        // 循环列表当前表头位置
        volatile size_t start_pos;
        // 循环列表下一次插入点位置（仅receive使用）
        /*volatile*/ size_t next_pos;

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

    }_m_recvDatas;
    
    /**
     * @brief 显示缓冲区相关参数。每次onSurfaceDraw的调用都会覆盖其中的内容
     * 除初始化外，本区域仅供draw读写
     */
    struct _rend_data_t
    {
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
        int16_t last_minmax;


        /* 波形幅值缩放控制区 结束 */

    }_m_renderDatas;

    /* 缓冲区初始化函数 开始*/
    // 清理接收缓冲区
    void _recv_dataReinit();
    // 清理绘制缓冲区
    void _render_dataReinit();
    /* 缓冲区初始化函数 结束*/
    
    // 复制缓冲区内容
    void _copy_from_receiever();

};

#endif