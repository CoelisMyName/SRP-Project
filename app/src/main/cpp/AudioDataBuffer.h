#ifndef SRP_PROJECT_AUDIODATABUFFER_H
#define SRP_PROJECT_AUDIODATABUFFER_H

#include <cstdint>

template<typename T>
class Queue {
public:
    explicit Queue(int32_t capacity = 10) {
        if (capacity < 0) {
            capacity = 0;
        }
        mCapacity = capacity + 1;
        mArray = new T[mCapacity];
        mHead = mRear = 0;
    }

    virtual ~Queue() {
        delete[] mArray;
    }

    bool push(T src) {
        if (full()) return false;
        mArray[mRear] = src;
        mRear = (mRear + 1) % mCapacity;
        return true;
    }

    bool poll(T &dst) {
        if (empty()) return false;
        dst = mArray[mHead];
        mHead = (mHead + 1) % mCapacity;
        return true;
    }

    bool reallocate(int32_t capacity) {
        int32_t size = Queue::size();
        if (capacity < size) return false;
        T *array = new T[capacity + 1];
        for (int32_t i = 0, j = mHead;
             j != mRear; i = (i + 1) % (capacity + 1), j = (j + 1) % mCapacity) {
            array[i] = mArray[j];
        }
        mHead = 0;
        mRear = size;
        delete[] mArray;
        mArray = array;
        mCapacity = capacity + 1;
    }

    int32_t capacity() {
        return mCapacity - 1;
    }

    int32_t size() {
        return (mRear - mHead + mCapacity) % mCapacity;
    }

    bool empty() {
        return mHead == mRear;
    }

    bool full() {
        return mHead == (mRear + 1) % mCapacity;
    }

    void clear() {
        mHead = mRear = 0;
    }

private:
    volatile T *mArray;
    volatile int32_t mCapacity;
    volatile int32_t mHead = 0;
    volatile int32_t mRear = 0;
};

/**
 * 音频数据缓冲容器
 * @tparam T
 */
template<typename T>
class AudioDataBuffer {
public:
    /**
     *
     * @param sample_rate 采样率
     * @param frame 每次读取数据帧大小
     * @param overlap 帧与帧之间重叠大小
     */
    AudioDataBuffer(int32_t sample_rate, int32_t frame, int32_t overlap);

    /**
     *
     * @param sample_rate 采样率
     * @param capacity 容器大小
     * @param frame 每次读取数据帧大小
     * @param overlap 帧与帧之间重叠大小
     */
    AudioDataBuffer(int32_t sample_rate, int32_t capacity, int32_t frame, int32_t overlap);

    virtual ~AudioDataBuffer();

    int32_t getFrameSize();

    int32_t getOverlap();

    int32_t size();

    int32_t capacity();

    /**
     * 写入数据
     * @param timestamp 毫秒时间戳
     * @param src 数据指针
     * @param size 数据长度
     * @return 数据写入大小
     */
    int32_t put(int64_t timestamp, T *src, int32_t size);

    /**
     * 读一帧数据
     * @param dst 存放数组指针
     * @param capacity 数组容量
     * @param timestamp 返回当前帧毫秒时间戳
     * @return 若容量不足返回 -1，若未有一帧数据则返回 0，若读到数据则返回帧大小
     */
    int32_t next(T *dst, int32_t capacity, int64_t &timestamp);

    /**
     * 当前有一帧数据
     * @return
     */
    bool ready();

    bool empty();

    bool full();

    /**
     * 清空数据
     */
    void clear();

private:
    volatile T *mBuffer;
    Queue<int64_t> mTimestamp;
    volatile int32_t mSampleRate;
    volatile int64_t mSampleCount = 0;
    volatile int32_t mCapacity;
    volatile int32_t mFrame;
    volatile int32_t mOverlap;
    volatile int32_t mHead = 0;
    volatile int32_t mRear = 0;
};

#include "AudioDataBufferImpl.h"

#endif