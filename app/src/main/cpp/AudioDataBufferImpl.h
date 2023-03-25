#ifndef SRP_PROJECT_AUDIODATABUFFERIMPL_H
#define SRP_PROJECT_AUDIODATABUFFERIMPL_H

#include <cassert>
#include <cstring>
#include <algorithm>
#include "AudioDataBuffer.h"

template<typename T>
AudioDataBuffer<T>::AudioDataBuffer(int32_t sample_rate, int32_t frame, int32_t overlap)
        : mTimestamp(4) {
    mSampleRate = sample_rate;
    frame = frame <= 0 ? 1 : frame;
    overlap = overlap < 0 ? 0 : overlap;
    overlap = overlap >= frame ? frame - 1 : overlap;
    int32_t capacity = 3 * (frame - overlap) + overlap;
    mFrame = frame;
    mOverlap = overlap;
    mCapacity = capacity + 1;
    mBuffer = new T[mCapacity];
    mHead = mRear = 0;
}

template<typename T>
AudioDataBuffer<T>::AudioDataBuffer(int32_t sample_rate, int32_t capacity, int32_t frame,
                                    int32_t overlap) {
    mSampleRate = sample_rate;
    frame = frame <= 0 ? 1 : frame;
    overlap = overlap < 0 ? 0 : overlap;
    capacity = capacity <= 0 ? 1 : capacity;
    capacity = capacity <= frame ? frame + 1 : capacity;
    overlap = overlap >= frame ? frame - 1 : overlap;
    mTimestamp.reallocate((capacity - overlap) / (frame - overlap) + 1);
    mFrame = frame;
    mOverlap = overlap;
    mCapacity = capacity + 1;
    mBuffer = new T[mCapacity];
    mHead = mRear = 0;
}

template<typename T>
AudioDataBuffer<T>::~AudioDataBuffer() {
    delete[] mBuffer;
}

template<typename T>
int32_t AudioDataBuffer<T>::getFrameSize() {
    return mFrame;
}

template<typename T>
int32_t AudioDataBuffer<T>::getOverlap() {
    return mOverlap;
}

template<typename T>
int32_t AudioDataBuffer<T>::size() {
    return (mRear - mHead + mCapacity) % mCapacity;
}

template<typename T>
int32_t AudioDataBuffer<T>::capacity() {
    return mCapacity - 1;
}

//TODO timestamp has some bug
template<typename T>
int32_t AudioDataBuffer<T>::put(int64_t timestamp, T *src, int32_t size) {
    if (full()) return 0;
    int32_t head = mRear;
    int32_t empty = (mHead - mRear + mCapacity - 1) % mCapacity;
    int32_t write = std::min(empty, size);
    int32_t rear = (head + write) % mCapacity;
    if (rear >= head) {
        memcpy((void *)(&mBuffer[head]), (const void *)src, write * sizeof(T));
    } else {
        memcpy((void *)(&mBuffer[head]), (const void *)src, (mCapacity - head) * sizeof(T));
        memcpy((void *)mBuffer, (const void *)src, (write - mCapacity + head) * sizeof(T));
    }
    auto skew = (int32_t) (mSampleCount % (mFrame - mOverlap));
    skew = ((mFrame - mOverlap) - skew) % (mFrame - mOverlap);
    double interval = 1000.0 / mSampleRate;
    for (int32_t i = skew; i < size; i += (mFrame - mOverlap)) {
        int64_t t = timestamp + (int64_t) (i * interval);
        mTimestamp.push(t);
    }
    mRear = rear;
    mSampleCount += write;
    return write;
}

template<typename T>
int32_t AudioDataBuffer<T>::next(T *dst, int32_t capacity, int64_t &timestamp) {
    if (capacity < mFrame) return -1;
    if (!ready()) return 0;
    int32_t front = mHead;
    int32_t rear = (mHead + mFrame) % mCapacity;
    if (rear >= front) {
        memcpy((void *)dst, (const void *)(&mBuffer[front]), mFrame * sizeof(T));
    } else {
        memcpy((void *)dst, (const void *)(&mBuffer[front]), (mCapacity - front) * sizeof(T));
        memcpy((void *)(&dst[mCapacity - front]), (const void *)mBuffer, rear * sizeof(T));
    }
    mHead = (mHead + mFrame - mOverlap) % mCapacity;
    assert(mTimestamp.poll(timestamp));
    return mFrame;
}

template<typename T>
bool AudioDataBuffer<T>::ready() {
    return size() >= mFrame;
}

template<typename T>
bool AudioDataBuffer<T>::empty() {
    return mHead == mRear;
}

template<typename T>
bool AudioDataBuffer<T>::full() {
    return mHead == (mRear + 1) % mCapacity;
}

template<typename T>
void AudioDataBuffer<T>::clear() {
    mHead = mRear = 0;
    mSampleCount = 0;
    mTimestamp.clear();
}

#endif