/**
 * 缓存池实现
 */
#include <cstring>
#include <algorithm>
#include "BufferPool.h"

template<typename T>
BufferPool<T>::BufferPool(uint32_t frame_size, uint32_t buffer_size, uint32_t redundancy) {
    frame_size = frame_size <= 1 ? 2 : frame_size;
    buffer_size = buffer_size < 1 ? 1 : buffer_size;
    redundancy = redundancy >= buffer_size ? buffer_size - 1 : redundancy;
    m_size = frame_size;
    m_pool = new Buffer<T>[m_size];
    m_rdd = redundancy;
    for (uint32_t i = 0; i < m_size; ++i) {
        m_pool[i] = {new T[buffer_size], buffer_size, 0};
    }
    m_buffer = new T[m_rdd];
    clear();
}

template<typename T>
BufferPool<T>::~BufferPool() {
    for (uint32_t i = 0; i < m_size; ++i) {
        delete[] m_pool[i].data;
    }
    delete[] m_pool;
    delete[] m_buffer;
}

template<typename T>
uint32_t BufferPool<T>::write(T *src, uint32_t length) {
    if (m_full) return 0;
    forward();
    uint32_t index = 0, remain = length, wte = 0;
    while (remain > 0) {
        auto dst = &m_pool[m_wt];
        uint32_t empty = dst->capacity - dst->position;
        if (empty == 0) {
            if (forward()) continue;
            else return wte;
        }
        uint32_t wrt = std::min(remain, empty);
        memcpy(&dst->data[dst->position], &src[index], sizeof(T) * wrt);
        wte += wrt;
        dst->position += wrt;
        index += wrt;
        remain -= wrt;
    }
    forward();
    return wte;
}

template<typename T>
bool BufferPool<T>::ready() {
    return m_wt != m_rd || m_full;
}

template<typename T>
bool BufferPool<T>::empty() {
    return m_wt == m_rd && !m_full;
}

template<typename T>
bool BufferPool<T>::full() {
    return m_full;
}

template<typename T>
T *BufferPool<T>::next(uint32_t &length) {
    length = 0;
    if (!ready()) return nullptr;
    m_full = false;
    auto curr = &m_pool[m_rd];
    m_rd = (m_rd + 1) % m_size;
    length = curr->capacity;
    curr->position = 0;
    return curr->data;
}

template<typename T>
T *BufferPool<T>::peek(uint32_t &length) {
    length = 0;
    if (!ready()) return nullptr;
    auto curr = &m_pool[m_rd];
    length = curr->capacity;
    return curr->data;
}

template<typename T>
void BufferPool<T>::clear() {
    m_full = m_buf_on = false;
    m_wt = m_rd = 0;
    for (uint32_t i = 0; i < m_size; ++i) {
        m_pool[i].position = 0;
    }
}

template<typename T>
bool BufferPool<T>::forward() {
    if (m_full) return false;
    auto curr = &m_pool[m_wt];
    if (m_wt != m_rd && m_buf_on) {
        m_buf_on = false;
        memcpy(curr->data, m_buffer, sizeof(T) * m_rdd);
        curr->position = m_rdd;
    }
    if (curr->position < curr->capacity) return false;
    m_buf_on = true;
    memcpy(m_buffer, &curr->data[curr->capacity - m_rdd], sizeof(T) * m_rdd);
    m_wt = (m_wt + 1) % m_size;
    m_full = m_wt == m_rd;
    return true;
}