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

WaveRender::WaveRender() : mMaxQueue(WAVE_RENDER_POINT_NUM), mMinQueue(WAVE_RENDER_POINT_NUM) {
    mAudioBufferCapacity = WAVE_RENDER_INPUT_SIZE + FRAME_SIZE;
    mAudioBuffer = new int16_t[mAudioBufferCapacity];
    mBufferCapacity = WAVE_RENDER_POINT_NUM;
    mMaxBuffer = new int16_t[mBufferCapacity];
    mMinBuffer = new int16_t[mBufferCapacity];
}

WaveRender::~WaveRender() {
    delete[] mAudioBuffer;
    delete[] mMaxBuffer;
    delete[] mMinBuffer;
}

void WaveRender::onAudioCallbackAttach() {
}

void WaveRender::onAudioDataStart(int64_t timestamp) {
}

void WaveRender::onAudioDataStop(int64_t timestamp) {
}

void WaveRender::onAudioDataReceive(int64_t timestamp, int16_t *data, int32_t length) {
    unique_lock<mutex> lock(mMutex);
    int32_t audioBufferSize = mAudioBufferSize;
    //复制到公共缓冲内存
    memcpy(&mAudioBuffer[audioBufferSize], data, sizeof(int16_t) * length);
    audioBufferSize += length;
    int32_t workBufferSize = WAVE_RENDER_INPUT_SIZE;
    int16_t workBuffer[workBufferSize];
    //本次计算产生多少个值
    int32_t total = audioBufferSize / workBufferSize;
    int16_t max = 0, min = 0;
    //确保队列有足够空间
    while (mMaxQueue.size() + total > mMaxQueue.capacity()) {
        mMaxQueue.pop(max);
        mMinQueue.pop(min);
    }
    //每次取workBufferSize大小的数据，排序获得线的上下端值
    for (int32_t i = 0; i + workBufferSize <= audioBufferSize; i += workBufferSize) {
        memcpy(workBuffer, &mAudioBuffer[i], sizeof(int16_t) * workBufferSize);
        std::sort(workBuffer, workBuffer + workBufferSize);
        //按照一定比值，获取线的上下端值
        max = workBuffer[int32_t(WAVE_RENDER_RATIO * workBufferSize)];
        min = workBuffer[int32_t((1.0 - WAVE_RENDER_RATIO) * workBufferSize)];
        mMaxQueue.push(max);
        mMinQueue.push(min);
    }
    int32_t remain = audioBufferSize % workBufferSize;
    //数值不足则重新放到缓冲区头，并更新大小
    memmove(mAudioBuffer, &mAudioBuffer[audioBufferSize - remain], sizeof(int16_t) * remain);
    audioBufferSize = remain;
    mAudioBufferSize = audioBufferSize;
}

void WaveRender::onAudioCallbackDetach() {
}

void WaveRender::onRenderAttach() {
}

void WaveRender::onSurfaceCreate(int32_t width, int32_t height) {
    mWidth = width;
    mHeight = height;
    if (!mInit) {
        bool ret;
        ret = loadProgramFromAssets("shader/wave.vert", "shader/wave.frag", mPgr);
        if (ret) {
            mInit = true;
        }
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        const static GLfloat vertices[] = {
                -0.5f, -0.5f, 0.0f,
                0.5f, -0.5f, 0.0f,
                0.0f, 0.5f, 0.0f
        };
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

void WaveRender::onSurfaceDraw() {
    int32_t bufferSize = mBufferSize;
    //同步区块
    {
        unique_lock<mutex> lock(mMutex);
        int32_t input = mMaxQueue.size();
        int32_t discard = 0;
        //检查输入数据与当前数据是否超出空间
        if (bufferSize + input > mBufferCapacity) {
            //丢弃数据长度
            discard = bufferSize + input - mBufferCapacity;
            //如果还有剩余数据，则移动到数组最前面
            if (bufferSize - discard > 0) {
                memmove(mMaxBuffer, &mMaxBuffer[discard], sizeof(int16_t) * (bufferSize - discard));
                memmove(mMinBuffer, &mMinBuffer[discard], sizeof(int16_t) * (bufferSize - discard));
                bufferSize -= discard;
                discard = 0;
            } else {
                discard -= bufferSize;
                bufferSize = 0;
            }
        }
        int16_t max, min;
        //如果还需要继续丢弃数据，则从容器里丢弃
        while (discard > 0 && !mMaxQueue.empty()) {
            mMaxQueue.pop(max);
            mMinQueue.pop(min);
            discard -= 1;
        }
        //向数组添加数据
        while (!mMaxQueue.empty()) {
            mMaxQueue.pop(mMaxBuffer[bufferSize]);
            mMinQueue.pop(mMinBuffer[bufferSize]);
            bufferSize += 1;
        }
        mBufferSize = bufferSize;
        lock.unlock();
    }

    glUseProgram(mPgr);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(mVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void WaveRender::onSurfaceSizeChange(int32_t width, int32_t height) {
    mWidth = width;
    mHeight = height;
}

void WaveRender::onSurfaceDestroy() {
    if (mInit) {
        glDeleteProgram(mPgr);
        mPgr = 0;
        mInit = false;
        glDeleteBuffers(1, &mVbo);
        glDeleteVertexArrays(1, &mVao);
        mVbo = 0;
        mVao = 0;
    }
}

void WaveRender::onRenderDetach() {
}