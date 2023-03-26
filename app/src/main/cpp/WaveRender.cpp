#include <cstring>
#include <algorithm>
#include <android/asset_manager.h>
#include "log.h"
#include "global.h"
#include "glesUtils.h"
#include "WaveRender.h"

TAG(WaveRender)

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MIN3(x, y, z) ((MIN((x), (y)) < (z)) ? MIN((x), (y)) : (z))

const static float _Color_Light_Background[4] = WAVERENDER_COLOR_LIGHT_BACKGROUND;
const static float _Color_Dark_Background[4] = WAVERENDER_COLOR_DARK_BACKGROUND;

const static float * const _Color_Backgrounds[2] =
        {_Color_Light_Background, _Color_Dark_Background};
#undef WAVERENDER_COLOR2F
#undef WAVERENDER_COLOR

#define WAVERENDER_PTR2PARAM(ptr)   ptr[0], ptr[1], ptr[2], ptr[3]
#define WAVERENDER_CUR_BG(thiz)     WAVERENDER_PTR2PARAM(_Color_Backgrounds[thiz->_m_isAtDarkMode])

/**
 * @brief 将 int16_t 型输入按对数规则映射。最大最小值仍为 [INT16_MIN, INT16_MAX]
 *
 * @param origin
 * @return int16_t
 */
int16_t log10_map(int16_t origin) {
    // 避免0异常
    if (origin == 0) {
        return 0;
    }
    const bool flip = origin < 0;
    if (flip)
        origin = -origin;

    double r = log10(origin) * INT16_MAX / 4.515;
    if (r > INT16_MAX) {
        r = INT16_MAX;
    }
    if (flip) {
        r = -r;
    }
    return (int16_t) r;
}

/**
 * @brief 波形强度映射.
 * 该函数应当保证（非严格）单调递增.
 * 在receive获取到端点值后使用.
 *
 * @param origin 原强度
 * @return int16_t
 */
int16_t strength_map(int16_t origin) {
    return int16_t((int32_t(origin) * 9 + int32_t(log10_map(origin))) / 10);
}

WaveRender::WaveRender() {
    // 清理接收缓冲区
    this->_recv_dataReinit();
    // 清理绘制缓冲区
    this->_render_dataReinit();
}

WaveRender::~WaveRender() {
}

void WaveRender::onAudioCallbackAttach() {
}

void WaveRender::onAudioDataStart(int64_t timestamp) {
    // 清理接收缓冲区
    this->_recv_dataReinit();
}

void WaveRender::onAudioDataStop(int64_t timestamp) {
}

void WaveRender::onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) {
    // 无效参数保护
    if ((length <= 0) || (nullptr == data)) {
        return;
    }
    // 读取上次运行结果，避免一直全局访存
    uint32_t group_pos = this->_m_recvDatas.cur_pos;
    int16_t group_max = 0, group_min = 0;
    // 读取插入点，避免频繁对插入点进行++
    size_t local_next_pos = this->_m_recvDatas.next_pos;
    if (group_pos) {
        group_max = this->_m_recvDatas.cur_max;
        group_min = this->_m_recvDatas.cur_min;
    }
    // 本次读取进度，用于跨循环同步
    int32_t in_pos = 0;
    do {
        // 读取缓冲区信息，得到可无锁安全写入的区间长度.
        // 在这一区间内运行时，无需上 mutex
        const size_t safety_len = WAVERENDER_BUFFER_SIZE - this->_m_recvDatas.available_length;
        // 统计本次循环写入数量的标记.
        // 一次插入操作后，当标记 == safety_len则应当进行缓冲区参数更新
        size_t len_increase = 0;
        // 进入外层循环.
        // 该循环将在写满安全区间后通过一个break退出.
        // 无论安全区间是否被写满，此循环退出后将紧跟缓冲区更新（带锁）
        while (in_pos < length) {
            // 内层循环.
            // 如果数据充足，循环退出时产生一对 min-max，并清空 this->_m_recvDatas.cur_*记录
            // 如果数据不足（in_pos == length 退出）则由外层循环更新 this->_m_recvDatas.cur_* 信息
            for (; (group_pos < WAVE_RENDER_INPUT_SIZE) &&
                   (in_pos < length); ++group_pos, ++in_pos) {
                // 比对
                group_max = (group_max >= data[in_pos]) ? group_max : data[in_pos];
                group_min = (group_min <= data[in_pos]) ? group_min : data[in_pos];
            }
            // 检查是因为一组已满还是输入用完
            if (group_pos == WAVE_RENDER_INPUT_SIZE) {
                // 一组数据已满，此时向缓冲区队尾插入并更新队尾坐标.
                // 在输入前进行强度映射
                this->_m_recvDatas.buffer[local_next_pos].maximun = strength_map(group_max);
                this->_m_recvDatas.buffer[local_next_pos].minimun = strength_map(group_min);
                ++local_next_pos;
                if (local_next_pos == WAVERENDER_BUFFER_SIZE) {
                    local_next_pos = 0;
                }
                // 清理各局部状态变量，这些状态的写回在外层循环进行
                group_max = group_min = group_pos = 0;
                // 写入次数自增
                ++len_increase;
                // 检查是否应当退出，去更新缓冲区参数
                if (len_increase == safety_len) {
                    break;
                }
            }
            // 数据未满的情况没有额外的处理，此时外层循环会自动退出
        }
        // this->_m_recvDatas.cur_*记录更新
        this->_m_recvDatas.cur_pos = group_pos;
        this->_m_recvDatas.cur_max = group_max;
        this->_m_recvDatas.cur_min = group_min;
        // 计算新的 available_length
        // 由于数据只由本接口写，此时仍不必上锁
        size_t new_al = this->_m_recvDatas.available_length + len_increase;
        if (new_al > WAVERENDER_MAX_WIDTH) {
            new_al = WAVERENDER_MAX_WIDTH;
        }
        // 带锁操作，更新关键的 available_length 与 next_pos
        // 此处不锁定也可在大部分情况下安全同步，但不锁定时两处赋值顺序不可交换
#ifndef WAVERENDER_SYNC_RISK
        this->mMutex.lock();
#endif
        this->_m_recvDatas.next_pos = local_next_pos;
        this->_m_recvDatas.available_length = new_al;
#ifndef WAVERENDER_SYNC_RISK
        this->mMutex.unlock();
#endif
    }
        // 还没有读取完，返回继续读取
    while (in_pos < length);
}

void WaveRender::onAudioCallbackDetach() {
}

void WaveRender::onRenderAttach() {
}

void WaveRender::onSurfaceCreate(int32_t width, int32_t height) {
    // 清理绘图缓冲区

    mWidth = width;
    mHeight = height;
    if (!mInit) {
        bool ret;
        // 两组着色器，分别用于深色与浅色模式
        ret = loadProgramFromAssets("shader/wave.vert", "shader/wave_light.frag", mPgrLight);
        if (ret) {
            ret = loadProgramFromAssets("shader/wave.vert", "shader/wave_dark.frag", mPgrDark);
            if (ret) {
                mInit = true;
            } else {
                return;
            }
        } else {
            return;
        }
        glClearColor(WAVERENDER_CUR_BG(this));
        glClear(GL_COLOR_BUFFER_BIT);
        mVbo = 0, mVao = 0;
        glGenVertexArrays(1, &mVao);
        glBindVertexArray(mVao);

        glGenBuffers(1, &mVbo);
        glBindBuffer(GL_ARRAY_BUFFER, mVbo);
        glEnableVertexAttribArray(0);
    }
}

void WaveRender::onSurfaceDraw() {
    // 复制数据
    this->_copy_from_receiver();
    // 清屏
    if (this->_m_isAtDarkMode) {
        glUseProgram(mPgrDark);
    } else {
        glUseProgram(mPgrLight);
    }
    glClearColor(WAVERENDER_CUR_BG(this));
    glClear(GL_COLOR_BUFFER_BIT);
    // 如果没有可用绘制数据，直接返回
    if (0 == this->_m_renderDatas.size) {
        return;
    }
    // 对波形高度再次进行处理
    this->_render_convert();
    // 将波形映射到绘制用的数组
    this->_render_genPoints();
    // 配置缓冲区
    glBindVertexArray(mVao);
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * this->_m_renderDatas.size * sizeof(GLfloat),
                 this->_m_renderDatas.output_buffer, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_LINES, 0, 2 * this->_m_renderDatas.size);
}

void WaveRender::onSurfaceSizeChange(int32_t width, int32_t height) {
    mWidth = width;
    mHeight = height;
}

void WaveRender::onSurfaceDestroy() {
    if (mPgrLight) {
        glDeleteProgram(mPgrLight);
        mPgrLight = 0;
    }
    if (mPgrDark) {
        glDeleteProgram(mPgrDark);
        mPgrDark = 0;
    }
    if (mInit) {
        mInit = false;
        glDeleteBuffers(1, &mVbo);
        glDeleteVertexArrays(1, &mVao);
        mVbo = 0;
        mVao = 0;
    }
}

void WaveRender::onRenderDetach() {
}

void WaveRender::onDarkModeChange(bool intoDarkMode) {
    this->_m_isAtDarkMode = intoDarkMode;
}

void WaveRender::_recv_dataReinit() {
    this->_m_recvDatas.available_length = 0;
    this->_m_recvDatas.next_pos = 0;
    for (size_t i = 0; i < WAVERENDER_BUFFER_SIZE; ++i) {
        this->_m_recvDatas.buffer[i].maximun = 0;
        this->_m_recvDatas.buffer[i].minimun = 0;
    }
    // 重置统计信息
    this->_m_recvDatas.cur_max = 0;
    this->_m_recvDatas.cur_min = 0;
    this->_m_recvDatas.cur_pos = 0;
}

void WaveRender::_render_dataReinit() {
    for (size_t i = 0; i < WAVERENDER_MAX_WIDTH; ++i) {
        this->_m_renderDatas.buffer[i].maximun = 0;
        this->_m_renderDatas.buffer[i].minimun = 0;
    }
    this->_m_renderDatas.size = 0;
    this->_m_renderDatas.last_minmax = WAVERENDER_SCALE_PROTECTED_HEIGHT;
}

void WaveRender::_copy_from_receiver() {
#ifndef WAVERENDER_SYNC_RISK
    // 上锁准备复制
    this->mMutex.lock();
#endif
    // 保存变量到本地
    size_t pos = this->_m_recvDatas.next_pos;
    const int32_t localw = this->mWidth;
    const size_t max_len = this->_m_recvDatas.available_length;
    // 安全检查
    if ((max_len <= 0) || (pos <= 0) || (max_len > WAVERENDER_MAX_WIDTH) ||
        (pos >= WAVERENDER_BUFFER_SIZE)) {
        this->_render_dataReinit();
        return;
    }
    // 计算真正需要复制的宽度数量: localw WAVERENDER_MAX_WIDTH 与 max_len 中的最小值
    const size_t len = MIN3(localw, WAVERENDER_MAX_WIDTH, max_len);
    // 计算复制起点
    pos = pos + WAVERENDER_BUFFER_SIZE - len;
    if (pos >= WAVERENDER_BUFFER_SIZE) {
        pos -= WAVERENDER_BUFFER_SIZE;
    }

    // 复制动作
    for (size_t i = 0; i < len; ++i, ++pos) {
        // 接收缓冲区指针复位
        if (pos >= WAVERENDER_BUFFER_SIZE) {
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

void WaveRender::_render_convert() {
    // 提取出最值，用于缩放变动. max2只是临时值（用于保存负值）
    int16_t max = 0;
    {
        int16_t max2 = 0;
        // 遍历取当前区间最值
        for (size_t i = 0; i < this->_m_renderDatas.size; ++i) {
            // 最大
            if (this->_m_renderDatas.buffer[i].maximun > max) {
                max = this->_m_renderDatas.buffer[i].maximun;
            }
            // 最小
            max2 = MIN(max2, this->_m_renderDatas.buffer[i].minimun);
        }
        // 将max2取反与max比较得到较大的峰值
        max2 = -max2;
        max = (max > max2) ? max : max2;
    }
    // 当 max 未达到 INT16_MAX 时，接下来的代码用于尝试控制视图内 y_max 为 max + step，对波形进行缩放
    // 期望值的计算，并使 max + step 位于区间 [protected, INT16_MAX] 内
    int16_t hope = INT16_MAX;
    // 防止溢出的上限比较
    // 本次比较后 hope 的取值范围在 [max + step, INT16_MAX]
    if (max < (INT16_MAX - WAVERENDER_SCALE_MAX_STEP)) {
        // 期望y_max低于上限，改为上限
        hope = max + WAVERENDER_SCALE_MAX_STEP;
    }
    // 比较下限
    if (hope < WAVERENDER_SCALE_PROTECTED_HEIGHT) {
        hope = WAVERENDER_SCALE_PROTECTED_HEIGHT;
    }
    // 计算偏差值
    int16_t delta = hope - this->_m_renderDatas.last_minmax;
    // 对偏差值进行修正，避免缩放过快
    // 并得到最终应当取得的渲染区上限
    if (delta < 0) {
        if ((-delta) > WAVERENDER_SCALE_MAX_STEP) {
            hope = this->_m_renderDatas.last_minmax - WAVERENDER_SCALE_MAX_STEP;
        }
    } else {
        if (delta > WAVERENDER_SCALE_MAX_STEP) {
            hope = this->_m_renderDatas.last_minmax + WAVERENDER_SCALE_MAX_STEP;
        }
    }
    // 存回
    const int16_t real_max = hope;
    this->_m_renderDatas.last_minmax = real_max;
    // 过滤
    for (size_t i = 0; i < this->_m_renderDatas.size; ++i) {
        // 防止溢出，溢出置为INT16_MAX/MIN
        int32_t i1 = this->_m_renderDatas.buffer[i].maximun * ((int32_t) INT16_MAX) / real_max;
        int32_t i2 = this->_m_renderDatas.buffer[i].minimun * ((int32_t) INT16_MAX) / real_max;
        // 对溢出值削峰
        this->_m_renderDatas.buffer[i].maximun = (int16_t) (MIN(INT16_MAX, i1));
        this->_m_renderDatas.buffer[i].minimun = (i2 < INT16_MIN) ? INT16_MIN : (int16_t(i2));
    }
}

#define IN(i) (this->_m_renderDatas.buffer[(i)])
#define X1(i) (this->_m_renderDatas.output_buffer[(6 * (i))])
#define Y1(i) (this->_m_renderDatas.output_buffer[(6 * (i) + 1)])
#define Z1(i) (this->_m_renderDatas.output_buffer[(6 * (i) + 2)])
#define X2(i) (this->_m_renderDatas.output_buffer[(6 * (i) + 3)])
#define Y2(i) (this->_m_renderDatas.output_buffer[(6 * (i) + 4)])
#define Z2(i) (this->_m_renderDatas.output_buffer[(6 * (i) + 5)])

void WaveRender::_render_genPoints() {
    // 保存宽高
    const int32_t localw = this->mWidth;
    // 坐标映射规则为：
    // y轴范围由 [INT16_MIN, INT16_MAX] 映射至 [-1.0f, 1.0f]
    // x轴范围由 [0, size] 映射至 [-1.0f, 1.0f]
    // x轴方向上，div 内波形向左对齐，div最大宽度 max_width，div在窗口内水平居中
    // 因此，x方向步进为 step = 2.0/localw;
    // x0取值为 -1.0f 或 -1.0f + (maxw-localw)/2*step

    // 计算x0相关内容
    const GLfloat stepx = 2.0f / localw;
    GLfloat x0 = -1.0f + stepx;
    if (localw > WAVERENDER_MAX_WIDTH) {
        x0 += (GLfloat) (WAVERENDER_MAX_WIDTH - localw) / 2.0f * stepx;
    }
    // 进行变换
    for (size_t i = 0; i < this->_m_renderDatas.size; ++i) {
        // x1与x2取值与当前x0相同
        X1(i) = X2(i) = x0;
        // y1与y2的运算非常简单, 除以最大值即可
        Y1(i) = IN(i).maximun / (GLfloat(INT16_MAX));
        Y2(i) = IN(i).minimun / (GLfloat(INT16_MAX));
        // z1与z2置0即可
        Z1(i) = 0.0f;
        Z2(i) = 0.0f;
        // 更新 x0
        x0 += stepx;
    }
}