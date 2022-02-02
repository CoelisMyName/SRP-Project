/**
 * @file WaveWriterLite.cpp
 * @author Galactic-Y (1425136616@qq.com)
 * @brief 波形(PCM)数据保存为文件的简单实现
 * @version 0.1
 * @date 2022-02-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "WaveWriterLite.h"
// 标准c接口文件操作
#include <cstdio>

const uint8_t WaveWriterLite::head[22] = {
    'R', 'I', 'F', 'F',
    0, 0, 0, 0, // filesize - 8
    'W', 'A', 'V', 'E',
    'f', 'm', 't', ' ',
    16, 0, 0, 0, // fmt块大小（uint32_t常数 16）
    1, 0         // 文件格式（PCM为uint16_t常量 1）
};
const uint8_t WaveWriterLite::data_head[8] = {'d', 'a', 't', 'a', 0, 0, 0, 0};

/**
 * @brief 按判定的字节序向本类的pFile写入一个基本数据.
 *
 * @warning 本操作不检查pFile状态.
 *
 * @note 建议只用基本数据类型，因为这里不使用引用
 *
 * @tparam T 数据类型
 * @param data 数据值
 * @retval true 写入失败
 * @retval false 成功写入
 */
template <typename T>
inline bool _write_endian(const T data, WaveWriterLite &wwl)
{
    FILE *const fp = (FILE *)wwl._pFile;
    const uint8_t *pdata = (uint8_t *)&data;
    // 大端模式
    if (wwl._big_endian)
    {
        for (int i = sizeof(T) - 1; i >= 0; --i)
        {
            if (fwrite(&pdata[i], 1, 1, fp) != 1)
            {
                return true;
            }
        }
    }
    // 小端模式
    else
    {
        for (int i = 0; i < sizeof(T); ++i)
        {
            if (fwrite(&pdata[i], 1, 1, fp) != 1)
            {
                return true;
            }
        }
    }
    // 没有中途return
    return false;
}

WaveWriterLite::WaveWriterLite(const char *const uri,
                               const uint32_t sample_rate,
                               const uint16_t num_channals,
                               const uint16_t bytes_per_sample)
{
    // 初始化成员为空
    this->_good = false;
    this->_pFile = NULL;
    // 参数检查
    if ((uri == NULL) || (sample_rate == 0) || (num_channals == 0) || (bytes_per_sample == 0))
    {
        this->_good = false;
        return;
    }
    // 检查大小端模式
    this->_big_endian = this->_check_endian();
    // 打开文件
    this->_pFile = (void *)fopen(uri, "wb+");
    // 检查打开结果
    if (this->_pFile == NULL)
    {
        return;
    }
    { // 写入文件头中固定的字节，检查是否写入成功
        size_t i = fwrite(this->head, 1, 22, (FILE *)this->_pFile);
        // 成功写入的字节不等于22说明中间存在写入失败的情况
        // 为了避免外部不知道是否需要close，在这里释放
        if (i != 22)
        {
            this->close();
            return;
        }
    }
    // 储存格式信息
    this->_frame_size = num_channals * bytes_per_sample;
#if WAVEWRITERLITE_WBYTE_FLIP
    this->_bytes_per_sample = bytes_per_sample;
#endif // WAVEWRITERLITE_WBYTE_FLIP
    // 开始写入fmt信息，有一次失败就终止
    // 声道数
    if (_write_endian<uint16_t>(num_channals, *this))
    {
        this->close();
        return;
    }
    // 采样率
    if (_write_endian<uint32_t>(sample_rate, *this))
    {
        this->close();
        return;
    }
    // 每秒数据字节数
    if (_write_endian<uint32_t>(num_channals * sample_rate * bytes_per_sample, *this))
    {
        this->close();
        return;
    }
    // 数据块对齐
    if (_write_endian<uint16_t>(num_channals * bytes_per_sample, *this))
    {
        this->close();
        return;
    }
    // 采样位数
    if (_write_endian<uint16_t>(8 * bytes_per_sample, *this))
    {
        this->close();
        return;
    }
    { // 写入data块头部，检查是否写入成功
        size_t i = fwrite(this->data_head, 1, 8, (FILE *)this->_pFile);
        // 成功写入的字节不等于8说明中间存在写入失败的情况
        // 为了避免外部不知道是否需要close，在这里释放
        if (i != 8)
        {
            this->close();
            return;
        }
    }
    // 到目前为止都成功了
    // 改为启用
    this->_good = true;
}

WaveWriterLite::~WaveWriterLite()
{
    // 调用close()释放资源（在close阶段检查状态）
    this->close();
}
uint32_t WaveWriterLite::write(const int8_t *pdata, const uint32_t length)
{
    // 检查state
    if (this->_good == false)
    {
        return 0;
    }
    // 检查pFile
    if (this->_pFile == NULL)
    {
        this->_good = false;
        return 0;
    }
    // 检查length对齐
    if (length % this->_frame_size != 0)
    {
        return 0;
    }
    // 进入 try - catch避免越界卡系统
    try
    {
#if WAVEWRITERLITE_WBYTE_FLIP
        // 如果每通道字节数为1则复用不翻转的逻辑
        if (this->_bytes_per_sample != 1)
        {
            // 外循环
            for (uint32_t i = -1; i < length - 1; i += this->_bytes_per_sample)
            {
                // 内循环
                for (uint32_t j = this->_bytes_per_sample; j > 0; --j)
                {
                    if (fwrite(&pdata[i + j], 1, 1, (FILE *)this->_pFile) != 1)
                    {
                        throw 1;
                    }
                }
            }
        }
        else
#endif // WAVEWRITERLITE_WBYTE_FLIP
        {
            uint32_t written = fwrite(pdata, 1, length, (FILE *)this->_pFile);
            // 写入出错
            if (written != length)
            {
                this->_good = false;
                return 0;
            }
        }
    }
    catch (...)
    {
        return 0;
    }
    return length;
}

bool WaveWriterLite::close()
{
    // 如果 pFile 非空才需关闭
    if (this->_pFile != NULL)
    {
        int i = fclose((FILE *)_pFile);
        this->_pFile = NULL;
        // 因为没有可输出文件了，因此状态为bad
        this->_good = false;
        // fclose正常为返回0
        return i == 0;
    }
    else
    {
        // 再次写入状态
        this->_good = false;
        return true;
    }
}

WaveWriterLite::operator bool()
{
    return this->good();
}

bool WaveWriterLite::good()
{
    // 再次触发检查
    if (this->_pFile == NULL)
    {
        this->_good = false;
    }
    return this->_good;
}
