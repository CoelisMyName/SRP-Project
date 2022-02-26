#include <cstring>
#include <algorithm>
#include <android/asset_manager.h>
#include "log.h"
#include "global.h"
#include "glesUtils.h"
#include "WaveRender.h"

TAG(WaveRender)

using std::mutex;
using std::unique_lock;

/**
 * @brief 波形强度映射.
 * 该函数应当保证（非严格）单调递增.
 * 在receieve获取到端点值后使用.
 *
 * @param origin 原强度
 * @return int16_t
 */
int16_t strength_map(int16_t origin)
{
    return origin;
}

WaveRender::WaveRender() : mMaxQueue(WAVE_RENDER_POINT_NUM), mMinQueue(WAVE_RENDER_POINT_NUM)
{
    mAudioBufferCapacity = WAVE_RENDER_INPUT_SIZE + FRAME_SIZE;
    mAudioBuffer = new int16_t[mAudioBufferCapacity];
    mBufferCapacity = WAVE_RENDER_POINT_NUM;
    mMaxBuffer = new int16_t[mBufferCapacity];
    mMinBuffer = new int16_t[mBufferCapacity];
    // 清理接收缓冲区
    this->_recv_dataReinit();
    // 清理绘制缓冲区
    this->_render_dataReinit();
}

WaveRender::~WaveRender()
{
    delete[] mAudioBuffer;
    delete[] mMaxBuffer;
    delete[] mMinBuffer;
}

void WaveRender::onAudioCallbackAttach()
{
}

void WaveRender::onAudioDataStart(int64_t timestamp)
{
    // 清理接收缓冲区
    this->_recv_dataReinit();
}

void WaveRender::onAudioDataStop(int64_t timestamp)
{
}

void WaveRender::onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length)
{
    // 无效参数保护
    if ((length <= 0) || (NULL == data))
    {
        return;
    }
    // 读取上次运行结果，避免一直全局访存
    uint32_t group_pos = this->_m_recvDatas.cur_pos;
    int16_t group_max = 0, group_min = 0;
    if (group_pos)
    {
        group_max = this->_m_recvDatas.cur_max;
        group_min = this->_m_recvDatas.cur_min;
    }
    // 本次读取进度，用于跨循环同步
    int32_t in_pos = 0;
    do
    {
        // 读取缓冲区信息，得到可无锁安全写入的区间长度.
        // 在这一区间内运行时，无需上 mutex
        const size_t safety_len = WAVERENDER_BUFFER_SIZE - this->_m_recvDatas.available_length;
        // 统计本次循环写入数量的标记.
        // 一次插入操作后，当标记 == safety_len则应当进行缓冲区参数更新
        size_t len_increase = 0;
        // 进入外层循环.
        // 该循环将在写满安全区间后通过一个break退出.
        // 无论安全区间是否被写满，此循环退出后将紧跟缓冲区更新（带锁）
        while (in_pos < length)
        {
            // 内层循环.
            // 如果数据充足，循环退出时产生一对 min-max，并清空 this->_m_recvDatas.cur_*记录
            // 如果数据不足（in_pos == length 退出）则由外层循环更新 this->_m_recvDatas.cur_* 信息
            for (; (group_pos < WAVE_RENDER_INPUT_SIZE) && (in_pos < length); ++group_pos, ++in_pos)
            {
                // 比对
                group_max = (group_max >= data[in_pos]) ? group_max : data[in_pos];
                group_min = (group_min <= data[in_pos]) ? group_min : data[in_pos];
            }
            // 检查是因为一组已满还是输入用完
            if (group_max == WAVE_RENDER_INPUT_SIZE)
            {
                // 一组数据已满，此时向缓冲区队尾插入并更新队尾坐标.
                // 在输入前进行强度映射
                this->_m_recvDatas.buffer[this->_m_recvDatas.next_pos].maximun = strength_map(group_max);
                this->_m_recvDatas.buffer[this->_m_recvDatas.next_pos].minimun = strength_map(group_min);
                ++this->_m_recvDatas.next_pos;
                // 清理各局部状态变量，这些状态的写回在外层循环进行
                group_max = group_min = group_pos = 0;
                // 写入次数自增
                ++len_increase;
                // 检查是否应当退出，去更新缓冲区参数
                if (len_increase == safety_len)
                {
                    break;
                }
            }
            // 数据未满的情况没有额外的处理，此时外层循环会自动退出
        }
        // this->_m_recvDatas.cur_*记录更新
        this->_m_recvDatas.cur_pos = group_pos;
        this->_m_recvDatas.cur_max = group_max;
        this->_m_recvDatas.cur_min = group_min;
        // 计算新的 available_length 与 start_pos
        // 由于数据只由本接口写，此时仍不必上锁
        size_t new_al = this->_m_recvDatas.available_length,
               new_sp = this->_m_recvDatas.start_pos;
        // 根据 len_increase 计算 start_pos的移动
        // 需要注意的是，如果队列未满，len_increase不一定导致队首移动一样的距离
        {
            // 由于mWidth可能改变因此需要缓存
            const int32_t mw = this->mWidth;
            // 计算宽度上限
            const size_t upper_bound = (mw > WAVERENDER_MAX_WIDTH) ? WAVERENDER_MAX_WIDTH : mw;
            // 计算长度是否需要增长.
            // overflow 为非负值时，sp向前移动overflow单位，al增加至upper_bound
            // overflow 为负时，al增加len_increase
            const size_t overflow = new_al + len_increase - upper_bound;
            if (overflow >= 0)
            {
                new_sp += overflow;
                if (new_sp >= WAVERENDER_BUFFER_SIZE)
                {
                    new_sp -= WAVERENDER_BUFFER_SIZE;
                }
                new_al = upper_bound;
            }
            else
            {
                new_al += len_increase;
            }
        }
        // 带锁操作，更新关键的 available_length 与 start_pos
        // 此处不锁定也可在大部分情况下安全同步，但不锁定时两处赋值顺序不可交换
#ifndef WAVERENDER_SYNC_RISK
        this->mMutex.lock();
#endif
        this->_m_recvDatas.start_pos = new_sp;
        this->_m_recvDatas.available_length = new_al;
#ifndef WAVERENDER_SYNC_RISK
        this->mMutex.unlock();
#endif
    }
    // 还没有读取完，返回继续读取
    while (in_pos < length);
}

void WaveRender::onAudioCallbackDetach()
{
}

void WaveRender::onRenderAttach()
{
}

void WaveRender::onSurfaceCreate(int32_t width, int32_t height)
{
    // 清理绘图缓冲区

    mWidth = width;
    mHeight = height;
    if (!mInit)
    {
        bool ret;
        ret = loadProgramFromAssets("shader/wave.vert", "shader/wave.frag", mPgr);
        if (ret)
        {
            mInit = true;
        }
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        const static GLfloat vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f, 0.5f, 0.0f};
        mVbo = 0, mVao = 0;
        glGenVertexArrays(1, &mVao);
        glBindVertexArray(mVao);

        glGenBuffers(1, &mVbo);
        glBindBuffer(GL_ARRAY_BUFFER, mVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(0);
    }
}

void WaveRender::onSurfaceDraw()
{
    // 复制数据
    this->_copy_from_receiever();
    // 还没改
    glUseProgram(mPgr);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(mVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void WaveRender::onSurfaceSizeChange(int32_t width, int32_t height)
{
    mWidth = width;
    mHeight = height;
}

void WaveRender::onSurfaceDestroy()
{
    if (mInit)
    {
        glDeleteProgram(mPgr);
        mPgr = 0;
        mInit = false;
        glDeleteBuffers(1, &mVbo);
        glDeleteVertexArrays(1, &mVao);
        mVbo = 0;
        mVao = 0;
    }
}

void WaveRender::onRenderDetach()
{
}

void WaveRender::_recv_dataReinit()
{
    for (size_t i = 0; i < WAVERENDER_BUFFER_SIZE; ++i)
    {
        this->_m_recvDatas.buffer[i].maximun = 0;
        this->_m_recvDatas.buffer[i].minimun = 0;
    }
    this->_m_recvDatas.available_length = 0;
    this->_m_recvDatas.start_pos = 0;
    this->_m_recvDatas.next_pos = 0;
    // 重置统计信息
    this->_m_recvDatas.cur_max = 0;
    this->_m_recvDatas.cur_min = 0;
    this->_m_recvDatas.cur_pos = 0;
}
void WaveRender::_render_dataReinit()
{
    for (size_t i = 0; i < WAVERENDER_MAX_WIDTH; ++I)
    {
        this->_m_renderDatas.buffer[i].maximun = 0;
        this->_m_renderDatas.buffer[i].minimun = 0;
    }
    this->_m_renderDatas.size = 0;
    this->_m_renderDatas.last_minmax = 0;
}

void WaveRender::_copy_from_receiever()
{
#ifndef WAVERENDER_SYNC_RISK
    // 上锁准备复制
    this->mMutex.lock();
#endif
    // 保存变量到本地
    size_t pos = this->_m_recvDatas.start_pos;
    const size_t len = this->_m_recvDatas.available_length;
    // 安全检查
    if ((len <= 0) || (pos <= 0) || (len > WAVERENDER_MAX_WIDTH) || (pos >= WAVERENDER_BUFFER_SIZE) || (len > this->mWidth))
    {
        this->_render_dataReinit();
        return;
    }
    // 复制动作
    for (size_t i = 0; i < len; ++i, ++pos)
    {
        // 接收缓冲区指针复位
        if (pos >= WAVERENDER_BUFFER_SIZE)
        {
            pos = 0;
        }
        this->_m_renderDatas.buffer[i].maximun = this->_m_recvDatas.buffer[pos].maximun;
        this->_m_renderDatas.buffer[i].minimun = this->_m_recvDatas.buffer[pos].minimun;
    }
    // 复制参数
    this->_m_renderDatas.size = len;

#ifndef WAVERENDER_SYNC_RISK
    // 复制结束，解锁
    this->mMutex.unlock();
#endif
}