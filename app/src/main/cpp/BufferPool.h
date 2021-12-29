#ifndef SRP_PROJECT_BUFFERPOOL_H
#define SRP_PROJECT_BUFFERPOOL_H

#include <cstdint>

/**
 * 缓存结构
 * @tparam T
 */
template<typename T>
struct Buffer {
    T *data;
    uint32_t capacity;
    uint32_t position;
};

/**
 * 缓存池，复用缓存的缓存池
 * @tparam T
 */
template<typename T>
class BufferPool {
public:
    /**
     * 缓存池构造函数
     * @param frame_size 缓存池大小
     * @param buffer_size 每个缓存的大小
     * @param redundancy 每个缓存冗余的数据量，后一个缓存的头部会保存前一个缓存尾部定长数据，如前一个缓存保存1 2 3 4，redundancy设为1，那么后一个缓存则是4 5 6 7
     */
    BufferPool(uint32_t frame_size, uint32_t buffer_size, uint32_t redundancy = 0);

    ~BufferPool();

    /**
     * 写数据
     * @param src 数据指针
     * @param length 数据长度
     * @return 写入的数据长度
     */
    uint32_t write(T *src, uint32_t length);

    /**
     * 是否有缓存准备读取
     * @return 取决于是否有缓存以写满
     */
    bool ready();

    /**
     * 缓存池是否为空
     * @return 没有可读取的缓存且当前写的缓存未满
     */
    bool empty();

    /**
     * 缓存池是否写满
     * @return 缓存池写满，所有缓存可读，已经不能写了
     */
    bool full();

    /**
     * 获取当前可读的缓存，同时读指针指向下一个缓存
     * @param length 可读缓存长度
     * @return 缓存指针
     */
    T *next(uint32_t &length);

    /**
     * 获取当前可读的缓存，同时读指针不变
     * @param length 可读缓存长度
     * @return 缓存指针
     */
    T *peek(uint32_t &length);

    /**
     * 清空所有缓存
     */
    void clear();

private:
    Buffer<T> *m_pool; ///缓存数组
    uint32_t m_size; ///缓存数组大小
    uint32_t m_wt = 0; ///写指针
    uint32_t m_rd = 0; ///读指针
    uint32_t m_rdd; ///冗余数据大小
    bool m_full; ///是否为满
    bool m_buf_on; ///是否要将m_buffer写回
    T *m_buffer; ///冗余数据缓冲

    /**
     * 读指针向前
     * @return 前移是否成功，失败则是因为缓存池已满
     */
    bool forward();
};

#include "BufferPoolImplement.h"

#endif
