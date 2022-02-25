/**
 * @file WaveWriter.cpp
 * @author Galactic-Y (1425136616@qq.com)
 * @brief 波形(PCM)数据保存为文件的简单实现
 * @version 0.3
 * @date 2022-02-25
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "WaveWriter.h"
// 标准c接口文件操作
#include <cstdio>

const uint8_t WaveWriter::_m_head[22] = {
    'R', 'I', 'F', 'F',
    0, 0, 0, 0, // filesize - 8
    'W', 'A', 'V', 'E',
    'f', 'm', 't', ' ',
    16, 0, 0, 0, // fmt块大小（uint32_t常数 16）
    1, 0         // 文件格式（PCM为uint16_t常量 1）
};
const uint8_t WaveWriter::_m_data_head[8] = {'d', 'a', 't', 'a', 0, 0, 0, 0};

/**
 * @brief 按判定的字节序向本类的pfile写入一个基本数据.
 *
 * @warning 本操作不检查pfile状态.
 *
 * @note 建议只用基本数据类型，因为这里不使用引用
 *
 * @tparam T 数据类型
 * @param data 数据值
 * @retval true 写入失败
 * @retval false 成功写入
 */
template <typename T>
inline bool _write_endian(const T data, WaveWriter &wwl)
{
    FILE *const fp = (FILE *)wwl._m_pfile;
    const uint8_t *pdata = (uint8_t *)&data;
    // 大端模式
    if (wwl._m_big_endian)
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

WaveWriter::WaveWriter(const char *const uri,
                       const uint32_t sample_rate,
                       const uint16_t num_channals,
                       const uint16_t bytes_per_sample)
{
    // 初始化成员为空
    this->_m_good = false;
    this->_m_pfile = NULL;
    // 参数检查
    if ((uri == NULL) || (sample_rate == 0) || (num_channals == 0) || (bytes_per_sample == 0))
    {
        this->_m_good = false;
        return;
    }
    // 检查大小端模式
    this->_m_big_endian = this->_check_endian();
    // 打开文件
    this->_m_pfile = (void *)fopen(uri, "wb+");
    // 检查打开结果
    if (this->_m_pfile == NULL)
    {
        return;
    }
    { // 写入文件头中固定的字节，检查是否写入成功
        size_t i = fwrite(this->_m_head, 1, 22, (FILE *)this->_m_pfile);
        // 成功写入的字节不等于22说明中间存在写入失败的情况
        // 为了避免外部不知道是否需要close，在这里释放
        if (i != 22)
        {
            this->close();
            return;
        }
    }
    // 储存格式信息
    this->_m_frame_size = num_channals * bytes_per_sample;
    this->_m_bytes_per_sample = bytes_per_sample;
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
        size_t i = fwrite(this->_m_data_head, 1, 8, (FILE *)this->_m_pfile);
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
    this->_m_good = true;
}

WaveWriter::~WaveWriter()
{
    // 调用close()释放资源（在close阶段检查状态）
    this->close();
}
uint32_t WaveWriter::write(const int8_t *pdata, const size_t length, flip_t reverse)
{
    // 检查state
    if (this->_m_good == false)
    {
        return 0;
    }
    // 检查pfile
    if (this->_m_pfile == NULL)
    {
        this->_m_good = false;
        return 0;
    }
    // 检查length与对齐
    if ((length <= 0) || (length % this->_m_frame_size != 0))
    {
        return 0;
    }
    // 进入 try - catch避免越界卡系统
    try
    {
        // 检验是否使用翻转的逻辑
        // 当且仅当字节数不为1、 (reverse==ALWAYS) 或 (reverse==AUTO且为大端模式)使用翻转逻辑
        if ((this->_m_bytes_per_sample != 1) && ((reverse == flip_t::ALWAYS) || ((reverse == flip_t::AUTO) && (this->_m_big_endian == true))))
        {
            // 外循环
            for (uint32_t i = -1; i < length - 1; i += this->_m_bytes_per_sample)
            {
                // 内循环
                for (uint32_t j = this->_m_bytes_per_sample; j > 0; --j)
                {
                    if (fwrite(&pdata[i + j], 1, 1, (FILE *)this->_m_pfile) != 1)
                    {
                        throw 1;
                    }
                }
            }
        }
        else
        {
            uint32_t written = fwrite(pdata, 1, length, (FILE *)this->_m_pfile);
            // 写入出错
            if (written != length)
            {
                this->_m_good = false;
                return 0;
            }
        }
        // 实时更新大小，检查是否出错。出错视为失败
        if(this->_update_size() != true){
            return 0;
        }
    }
    catch (...)
    {
        return 0;
    }
    return length;
}

bool WaveWriter::close()
{
    // 如果 pfile 非空才需关闭
    if (this->_m_pfile != NULL)
    {
        // 再次更新文件大小
        this->_update_size();
        // 尝试关闭文件
        int i = fclose((FILE *)this->_m_pfile);
        this->_m_pfile = NULL;
        // 因为没有可输出文件了，因此状态为bad
        this->_m_good = false;
        // fclose正常为返回0
        return i == 0;
    }
    else
    {
        // 再次写入状态
        this->_m_good = false;
        return true;
    }
}

WaveWriter::operator bool()
{
    return this->good();
}

bool WaveWriter::good()
{
    // 再次触发检查
    if (this->_m_pfile == NULL)
    {
        this->_m_good = false;
    }
    return this->_m_good;
}

bool WaveWriter::_check_endian()
{
    const uint32_t i = 1;
    // 取变量低地址
    const uint8_t *const pi = (uint8_t *)&i;
    // 低地址为1, 说明为小端模式
    if (*pi)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool WaveWriter::_update_size()
{
    // 如果 pfile 非空才需更新
    if (this->_m_pfile != NULL)
    {
        // 保存文件原始指针
        int32_t oldp = ftell((FILE *)this->_m_pfile);
        // 获取文件大小
        // fseek失败会返回非0，把错误传递出去
        if (fseek((FILE *)this->_m_pfile, 0, SEEK_END) != 0)
        {
            return false;
        }
        int32_t sz = ftell((FILE *)this->_m_pfile);
        // 获取失败（-1），或大小异常（<44）则略过此阶段
        if (sz >= 44)
        {
            // 可以更新文件大小信息
            // riff信息中的大小
            fseek((FILE *)this->_m_pfile, 4L, SEEK_SET);
            _write_endian<int32_t>(sz - 8, *this);
            // data块中的大小
            fseek((FILE *)this->_m_pfile, 0x28, SEEK_SET);
            _write_endian<int32_t>(sz - 44, *this);
            // 恢复指针并返回
            return fseek((FILE *)this->_m_pfile, oldp, SEEK_SET) == 0;
        }
        else
        {
            // 失败
            return false;
        }
    }
    // 未打开文件，认为出错
    else
    {
        return false;
    }
}

int main(){
    WaveWriter w("./test.wav",6000,2,2);
    int16_t x[200] = {0};
    for(int i = 0;i<200;i+=2){
        x[i] = 100*i;
        x[i+1] = -100*i;
    }
    printf("%d",w.write((int8_t*)x,400));
    printf("%d",w.write((int8_t*)x,400));
    w.close();
    return 0;
}