/**
 * @file WaveWriter.cpp
 * @author Galactic-Y (1425136616@qq.com)
 * @brief 波形(PCM)数据保存为文件的实现
 * @version 0.1
 * @date 2022-01-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "WaveWriter.h"
// 标准c接口文件操作
#include <cstdio>

constexpr uint8_t num_of_ch(WaveWriter::CHANNAL chid)
{
    switch (chid)
    {
    case WaveWriter::CHANNAL::CH0:
    case WaveWriter::CHANNAL::CH1:
        return 1;
    case WaveWriter::CHANNAL::MULTI:
        return 2;
    default:
        return 0;
    }
}

const uint8_t WaveWriter::head[44] = {'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ',
                                      16, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 'd', 'a', 't', 'a', 0, 0, 0, 0};

WaveWriter::WaveWriter(uint32_t sample_rate, FORMAT format)
{
    // 重置指针及缓冲区信息
    this->_pfile = NULL;
    this->_pbuff = NULL;
    this->_buff_pdata = NULL;
    this->_buff_end = NULL;
    this->_which_ch = CHANNAL::MULTI;
    this->_left_size = 0;
    this->_data_size = 0;
    this->_bad = false;
    // 调用格式设置方法
    this->set_format(sample_rate, format);
}
WaveWriter::~WaveWriter()
{
    if (this->_pfile)
    {
        this->close();
    }
    if (this->_pbuff)
    {
        delete[] this->_pbuff;
    }
}

uint32_t WaveWriter::get_sample_rate() const
{
    return this->_sample_rate;
}

bool WaveWriter::set_sample_rate(uint32_t sample_rate)
{
    // 检查文件打开情况
    if (this->_pfile)
    {
        return false;
    }
    this->_sample_rate = (sample_rate = 0) ? 1 : sample_rate;
    return true;
}

WaveWriter::FORMAT WaveWriter::get_format() const
{
    return this->_origin_fmt;
}

bool WaveWriter::set_format(uint32_t sample_rate, FORMAT format)
{
    // 检查文件打开情况
    if (this->_pfile)
    {
        return false;
    }
    // 设置采样率
    if (!set_sample_rate(sample_rate))
    {
        return false;
    }
    // 保存初始格式
    this->_origin_fmt = format;
    // 拆分格式
    switch (format)
    {
    case FORMAT::CH1_BIT16:
        this->_dual_ch = false;
        this->_dual_byte = true;
        break;
    case FORMAT::CH1_BIT8:
        this->_dual_ch = false;
        this->_dual_byte = false;
        break;
    case FORMAT::CH2_BIT16:
        this->_dual_ch = true;
        this->_dual_byte = true;
        break;
    case FORMAT::CH2_BIT8:
        this->_dual_ch = true;
        this->_dual_byte = false;
        break;
    default:
        this->_dual_ch = false;
        this->_dual_byte = false;
        break;
    }
    return true;
}

bool WaveWriter::set_format(const WaveWriter &ref)
{
    // 套娃实现
    return this->set_format(ref._sample_rate, ref._origin_fmt);
}

bool WaveWriter::open(const char *const uri)
{
    // 空uri或已有打开的文件，则打开失败
    if ((uri == NULL) || (this->_pfile != NULL))
    {
        return false;
    }
    this->_pfile = (void *)fopen(uri, "wb+");
    // 检查打开结果
    if (this->_pfile == NULL)
    {
        return false;
    }
    // 重置文件大小统计
    this->_data_size = 0;
    // 重置状态变量
    this->_bad = false;
    // 初始化文件头，该函数会同时设置 fmt 块中的信息
    this->_file_head_init();
    // 结束，返回结果
    return true;
}

bool WaveWriter::open(const char *const uri, uint32_t sample_rate, FORMAT format)
{
    // 设置格式，失败时终止打开动作
    if (!this->set_format(sample_rate, format))
    {
        return false;
    }
    // 按常规 open() 处理
    return this->open(uri);
}

uint32_t WaveWriter::write(const int8_t *pdata, const uint32_t length, CHANNAL ch)
{
    // 检查空指针
    if ((this->_pfile == NULL) || (pdata == NULL))
    {
        return 0;
    }
    // 检查bad
    if (this->_bad)
    {
        return 0U;
    }
    // 统计成功写入的字节数（也是下一个要写入的位置）
    uint32_t written_cnt = 0;
    // 避免pdata越界异常导致中止，进入 try - catch 块再访问 pdata
    try
    {
        // 双通道模式需要检查ch选项与缓冲区
        if (this->_dual_ch)
        {
            // ch 选中2个通道
            if (ch == CHANNAL::MULTI)
            {
                // 触发一次 flush()
                flush();
                // 检查length是否为 2*num_byte的整数倍，否的话拒绝输出
                {
                    uint32_t i = 2;
                    if (this->_dual_byte)
                    {
                        i = 4;
                    }
                    if (length % i != 0)
                    {
                        return 0;
                    }
                }
// 检查字节翻转选项，无需翻转则可以正常使用fwrite
#if WAVEWRITER_WBYTE_FLIP
                for (written_cnt = 0U; written_cnt < length;)
                {
                    // 使用返回值判别写入是否成功，如果写入失败则应当立即终止写入. 下同.
                    // （throw 一个任意值到catch块）
                    if (!fwrite(&pdata[written_cnt + 1U], 1, 1, (FILE *)this->_pfile))
                    {
                        throw 1;
                    }
                    ++written_cnt;
                    if (!fwrite(&pdata[written_cnt], 1, 1, (FILE *)this->_pfile))
                    {
                        throw 1;
                    }
                    ++written_cnt;
                }
#else
                written_cnt = (uint32_t)fwrite(pdata, 1, length, (FILE *)this->_pfile);
#endif // WAVEWRITER_WBYTE_FLIP
            }
            // 输入为单通道模式
            else
            {
                // 检查length是否为 num_byte的整数倍，否的话拒绝输出
                if (this->_dual_byte)
                {
                    if (length & 1)
                    {
                        return 0;
                    }
                }
                // 检查缓冲区与ch的对应情况
                // 如果向同一侧通道输出，则把pdata输出到对应缓冲区
                // （构造新的缓冲区，合并，替换）
                if (this->_which_ch == ch)
                {
                    uint32_t new_size = this->_left_size + length;
                    // 这里可能new失败或产生越界，额外的一层 try - catch避免替换阶段越界导致内存泄露
                    int8_t *pbuff_new = NULL;
                    try
                    {
                        pbuff_new = new int8_t[new_size];

                        // 把旧缓冲区的内容拷贝到新缓冲区
                        for (size_t i = 0; i < (size_t)_left_size; ++i)
                        {
                            pbuff_new[i] = (this->_buff_pdata)[i];
                        }
                        // 把新输入的波形流格式化，放入缓冲区
                        {
                            int8_t *p_concat = pbuff_new + _left_size;
                            // 检查字节翻转选项
#if WAVEWRITER_WBYTE_FLIP
                            // 16bit 模式为翻转追加
                            if (this->_dual_byte)
                            {
                                for (uint32_t i = 0; i < length; i += 2)
                                {
                                    p_concat[i] = pdata[i + 1];
                                    p_concat[i + 1] = pdata[i];
                                }
                            }
                            // 8bit复用简单追加
                            else
#endif // WAVEWRITER_WBYTE_FLIP
                            {
                                // 简单追加
                                for (uint32_t i = 0; i < length; ++i)
                                {
                                    p_concat[i] = pdata[i];
                                }
                            }
                        }
                        // 释放旧缓冲区
                        delete[](this->_pbuff);
                        // 缓冲区参数更新
                        this->_pbuff = this->_buff_pdata = pbuff_new;
                        this->_buff_end = pbuff_new + new_size;
                        this->_left_size = new_size;
                        // 已写入数量更新
                        written_cnt = length;
                    }
                    catch (...)
                    {
                        this->_bad = true;
                        // 如果是复制阶段出错，需要释放新缓冲区
                        if (pbuff_new != NULL)
                        {
                            delete[] pbuff_new;
                        }
                        return 0;
                    }
                }
                // 缓冲区空，则向缓冲区输出
                else if (this->_which_ch == CHANNAL::MULTI)
                {
                    int8_t *pbuff_new = NULL;
                    try
                    {
                        pbuff_new = new int8_t[length];
                        // 检查字节翻转选项
#if WAVEWRITER_WBYTE_FLIP
                        // 16bit 模式为翻转追加
                        if (this->_dual_byte)
                        {
                            for (uint32_t i = 0; i < length; i += 2)
                            {
                                pbuff_new[i] = pdata[i + 1];
                                pbuff_new[i + 1] = pdata[i];
                            }
                        }
                        // 8bit复用简单追加
                        else
#endif // WAVEWRITER_WBYTE_FLIP
                        {
                            // 简单追加
                            for (uint32_t i = 0; i < length; ++i)
                            {
                                pbuff_new[i] = pdata[i];
                            }
                        }
                        // 缓冲区参数更新
                        this->_pbuff = this->_buff_pdata = pbuff_new;
                        this->_buff_end = pbuff_new + length;
                        this->_left_size = length;
                        this->_which_ch = ch;
                        // 已写入数量更新
                        written_cnt = length;
                    }
                    catch (...)
                    {
                        this->_bad = true;
                        // 如果是复制阶段出错，需要释放新缓冲区
                        if (pbuff_new != NULL)
                        {
                            delete[] pbuff_new;
                        }
                        return 0;
                    }
                }
                // 向不同侧输出，则把缓冲区的交替输出
                else
                {
                    /* 流程：
                    1 创建一个新的缓冲区（大小为left+length）
                    2 对于二者重合的帧共有m帧，输出在前 2m * size 的位置
                    3 对于某个通道多出来的帧，追加在后方
                    4 释放旧缓冲区
                    5 fwrite前 2m * size
                    6 如果两个通道同步(2m * size == left+length)，则删除新缓冲区，否则挂入
                    */
                    // 新缓冲区指针
                    int8_t *pbuff_new = NULL;
                    try
                    {
                        // 1 创建一个新的缓冲区（大小为left+length）
                        pbuff_new = new int8_t[(this->_left_size) + length];
                        // 2 对于二者重合的帧共有m帧，输出在前 2m * size 的位置
                        uint32_t common = ((this->_left_size) < length) ? (this->_left_size) : length;
                        {
                            // 从格式得到输出的i0
                            uint32_t i0 = 1, old_i0 = 0, new_i0 = 0;
                            if (this->_dual_byte)
                                i0 = 2;
                            // 识别通道，得到新旧数据的初始偏移
                            if (_which_ch == CHANNAL::CH0)
                            {
                                new_i0 = i0;
                            }
                            else
                            {
                                old_i0 = i0;
                            }
                            // 根据是否为双字节复制
                            if (this->_dual_byte)
                            {
                                for (uint32_t j = 0; j < common; j += 2, old_i0 += 4, new_i0 += 4)
                                {
                                    // 旧缓存内原有内容
                                    pbuff_new[old_i0] = (this->_pbuff)[j];
                                    pbuff_new[old_i0 + 1] = (this->_pbuff)[j + 1];
                                    // 新波形内容
// 翻转宏
#if WAVEWRITER_WBYTE_FLIP
                                    pbuff_new[new_i0] = pdata[j + 1];
                                    pbuff_new[new_i0 + 1] = pdata[j];
#else
                                    pbuff_new[new_i0] = pdata[j];
                                    pbuff_new[new_i0 + 1] = pdata[j + 1];
#endif // WAVEWRITER_WBYTE_FLIP
                                }
                            }
                            else
                            {
                                for (uint32_t j = 0; j < common; ++j, old_i0 += 2, new_i0 += 2)
                                {
                                    // 旧缓存内原有内容
                                    pbuff_new[old_i0] = (this->_pbuff)[j];
                                    // 新波形内容
                                    pbuff_new[new_i0] = pdata[j];
                                }
                            }
                        }
                        // 3 对于某个通道多出来的帧，追加在后方
                        {
                            int8_t *const pconcat = pbuff_new + common;
                            // 因为common必为old或length之一，所以直接用for
                            for (uint32_t i = common; i < this->_left_size; ++i)
                            {
                                pconcat[i] = (this->_buff_pdata)[i];
                            }
                            for (uint32_t i = common; i < length; ++i)
                            {
                                pconcat[i] = pdata[i];
                            }
                        }
                        // 4 释放旧缓冲区（含信息），旧信息缓冲一份用于6
                        const uint32_t old_left = this->_left_size;
                        const CHANNAL old_ch = this->_which_ch;
                        delete[] this->_pbuff;
                        this->_pbuff = this->_buff_pdata = this->_buff_end = NULL;
                        this->_which_ch = CHANNAL::MULTI;
                        this->_left_size = 0U;
                        // 5 fwrite前 2m * size
                        if (fwrite(pbuff_new, 1, (size_t)(2 * common), (FILE *)this->_pfile) == (size_t)(2 * common))
                        {
                            written_cnt = length;
                        }
                        else
                        {
                            throw 1;
                        }
                        // 6 如果两个通道同步(2m * size == left+length)，则删除新缓冲区，否则挂入
                        if (old_left == length)
                        {
                            delete[] pbuff_new;
                        }
                        else
                        {
                            this->_pbuff = pbuff_new;
                            this->_buff_end = pbuff_new + old_left + length;
                            this->_buff_pdata = pbuff_new + 2 * common;
                            this->_left_size = old_left + length - 2 * common;
                            if (old_left > length)
                            {
                                this->_which_ch = old_ch;
                            }
                            else
                            {
                                this->_which_ch = ch;
                            }
                        }
                    }
                    catch (...)
                    {
                        this->_bad = true;
                        // 如果是复制阶段出错，需要释放新缓冲区
                        if (pbuff_new != NULL)
                        {
                            delete[] pbuff_new;
                        }
                        return 0;
                    }
                }
            }
        }
        // 对于单通道输出的文件，直接莽
        else
        {
            // 如果是16bit模式，检查字节数是否为偶数，否则拒绝输出
            if (this->_dual_byte)
            {
                // 检查字节数是否为偶数
                if (length & 1U)
                {
                    return 0;
                }
                else
                {
// 检查字节翻转选项，无需翻转则可以正常使用fwrite
#if WAVEWRITER_WBYTE_FLIP
                    for (written_cnt = 0U; written_cnt < length;)
                    {
                        // 使用返回值判别写入是否成功，如果写入失败则应当立即终止写入. 下同.
                        // （throw 一个任意值到catch块）
                        if (!fwrite(&pdata[written_cnt + 1U], 1, 1, (FILE *)this->_pfile))
                        {
                            throw 1;
                        }
                        ++written_cnt;
                        if (!fwrite(&pdata[written_cnt], 1, 1, (FILE *)this->_pfile))
                        {
                            throw 1;
                        }
                        ++written_cnt;
                    }
#else
                    written_cnt = (uint32_t)fwrite(pdata, 1, length, (FILE *)this->_pfile);
#endif // WAVEWRITER_WBYTE_FLIP
                }
            }
            // 8bit+1ch 模式, 无需考虑翻转，可直接输出
            else
            {
                written_cnt = (uint32_t)fwrite(pdata, 1, length, (FILE *)this->_pfile);
            }
        }
    }
    catch (...)
    {
        this->_bad = true;
        return written_cnt;
    }
    this->_data_size += written_cnt;
    return written_cnt;
}

uint32_t WaveWriter::writew(const int16_t *pdata, const uint32_t length, CHANNAL ch)
{
    // 目前, writew 按 write() 实现.
    // 检查字节数是否溢出
    if (length >= (1U << 31))
    {
        return 0U;
    }
    return this->write((const int8_t *)pdata, length * 2U, ch);
}

bool WaveWriter::flush(bool use_zero)
{
    // 未打开文件则是失败
    if (this->_pfile == NULL)
    {
        return false;
    }
    // bad 必失败
    if (this->_bad)
    {
        return false;
    }
    // 空缓冲区必成功
    if (this->_which_ch == CHANNAL::MULTI)
    {
        return true;
    }
    // 缓冲区被占用
    // 按照正常程序逻辑，只能为2通道模式+只有一个通道有数据
    // 由缓冲区的规定，无需判别翻转/奇数字节，输出就可以了

    // 步长的映射
    uint32_t step = 1;
    if (this->_dual_byte)
    {
        ++step;
    }
    try
    {
        // 输出0模式需要考虑哪个通道使用了缓冲区
        if (use_zero)
        {
            uint8_t zeros[2] = {0};
            if (this->_which_ch == CHANNAL::CH0)
            {
                // CH0 有数据，CH1的位置使用0
                for (; this->_buff_pdata < this->_buff_end; this->_buff_pdata += step)
                {
                    // 检查是否所有字节成功输出
                    if (fwrite(this->_buff_pdata, 1, step, (FILE *)this->_pfile) != (size_t)step)
                        throw 1;
                    if (fwrite(zeros, 1, step, (FILE *)this->_pfile) != (size_t)step)
                        throw 1;
                }
            }
            else
            {
                // CH1 有数据，CH0的位置使用0
                for (; this->_buff_pdata < this->_buff_end; this->_buff_pdata += step)
                {
                    // 检查是否所有字节成功输出
                    if (fwrite(zeros, 1, step, (FILE *)this->_pfile) != (size_t)step)
                        throw 1;
                    if (fwrite(this->_buff_pdata, 1, step, (FILE *)this->_pfile) != (size_t)step)
                        throw 1;
                }
            }
        }
        // 复制模式不需要
        else
        {
            for (; this->_buff_pdata < this->_buff_end; this->_buff_pdata += step)
            {
                // 检查是否所有字节成功输出
                if (fwrite(this->_buff_pdata, 1, step, (FILE *)this->_pfile) != (size_t)step)
                    throw 1;
                if (fwrite(this->_buff_pdata, 1, step, (FILE *)this->_pfile) != (size_t)step)
                    throw 1;
            }
        }
    }
    catch (...)
    {
        this->_bad = true;
    }
    // 无论是否正常运行，都更新统计数据，释放缓冲区
    this->_data_size += this->_left_size * 2;
    this->_left_size = 0;
    this->_which_ch = CHANNAL::MULTI;
    delete[] this->_pbuff;
    this->_pbuff = this->_buff_end = this->_buff_pdata = NULL;
    // 真正flush并返回
    return fflush((FILE *)this->_pfile) != EOF;
}

bool WaveWriter::close()
{
    // 如果未打开则成功
    if (this->_pfile == NULL)
    {
        return true;
    }
    // 如果为bad则不更新size等数据
    if (!this->_bad)
    {
        // 冲洗缓冲区
        this->flush();
        // 更新size信息
        this->_write_data_size();
        this->_write_riff_size();
    }
    // 关闭文件
    fclose((FILE *)this->_pfile);
    this->_pfile = NULL;
    // 处理缓冲区信息
    this->_left_size = 0;
    this->_which_ch = CHANNAL::MULTI;
    if (this->_pbuff != NULL)
    {
        delete[] this->_pbuff;
    }
    this->_pbuff = this->_buff_end = this->_buff_pdata = NULL;
    // 重置统计信息
    this->_bad = false;
    this->_data_size = 0U;
    return true;
}

void WaveWriter::_write_riff_size() {}

void WaveWriter::_write_data_size() {}

void WaveWriter::_write_num_channals()
{
    // 保存位置
    const long last_pos = ftell((FILE *)this->_pfile);
    fseek((FILE *)this->_pfile, 0x16, SEEK_SET);
    // 初始化为1通道
    uint8_t i = 8;
    // 看格式是否要*2
    if (this->_dual_ch)
    {
        i *= 2;
    }
    // 写入
    fwrite(&i, 1, 1, (FILE *)this->_pfile);
    // 填充
    i = 0;
    fwrite(&i, 1, 1, (FILE *)this->_pfile);
    fseek((FILE *)this->_pfile, last_pos, SEEK_SET);
}

void WaveWriter::_write_sample_rate()
{
    // 保存位置
    const long last_pos = ftell((FILE *)this->_pfile);
    fseek((FILE *)this->_pfile, 0x18, SEEK_SET);
    // 提一个指针
    uint8_t *psp = (uint8_t *)&(this->_sample_rate);
    // 写入, 对反转的判别在i从0到3还是3到0
#if WAVEWRITER_WBYTE_FLIP
    for (int i = 3; i >= 0; --i)
#else
    for (int i = 0; i < 4; ++i)
#endif // WAVEWRITER_WBYTE_FLIP
    {
        fwrite(&psp + i, 1, 1, (FILE *)this->_pfile);
    }
    fseek((FILE *)this->_pfile, last_pos, SEEK_SET);
}

void WaveWriter::_write_byte_rate()
{
    // 保存位置
    const long last_pos = ftell((FILE *)this->_pfile);
    fseek((FILE *)this->_pfile, 0x1C, SEEK_SET);
    uint32_t btr = this->_sample_rate;
    // 看格式是否要*2
    if (this->_dual_byte)
    {
        btr *= 2;
    }
    if (this->_dual_ch)
    {
        btr *= 2;
    }
    // 提一个指针
    uint8_t *pbtr = (uint8_t *)&btr;
    // 写入, 对反转的判别在i从0到3还是3到0
#if WAVEWRITER_WBYTE_FLIP
    for (int i = 3; i >= 0; --i)
#else
    for (int i = 0; i < 4; ++i)
#endif // WAVEWRITER_WBYTE_FLIP
    {
        fwrite(&pbtr + i, 1, 1, (FILE *)this->_pfile);
    }
    fseek((FILE *)this->_pfile, last_pos, SEEK_SET);
}

void WaveWriter::_write_block_alian()
{
    // 保存位置
    const long last_pos = ftell((FILE *)this->_pfile);
    fseek((FILE *)this->_pfile, 0x20, SEEK_SET);
    // 初始化为1字节
    uint8_t i = 8;
    // 看格式是否要*2
    if (this->_dual_byte)
    {
        i *= 2;
    }
    if (this->_dual_ch)
    {
        i *= 2;
    }
    // 写入
    fwrite(&i, 1, 1, (FILE *)this->_pfile);
    // 填充
    i = 0;
    fwrite(&i, 1, 1, (FILE *)this->_pfile);
    fseek((FILE *)this->_pfile, last_pos, SEEK_SET);
}

void WaveWriter::_write_bits_per_sample()
{
    // 保存位置
    const long last_pos = ftell((FILE *)this->_pfile);
    fseek((FILE *)this->_pfile, 0x22, SEEK_SET);
    // 准备写入比特数(初始化为8bit)
    uint8_t i = 8;
    // 看格式是否要改为16
    if (this->_dual_byte)
    {
        i = 16;
    }
    // 写入
    fwrite(&i, 1, 1, (FILE *)this->_pfile);
    // 填充
    i = 0;
    fwrite(&i, 1, 1, (FILE *)this->_pfile);
    fseek((FILE *)this->_pfile, last_pos, SEEK_SET);
}

void WaveWriter::_file_head_init()
{
    // 初始化头
    fwrite(this->head, 1, sizeof(this->head), (FILE *)this->_pfile);
    // 设置 fmt 块中的信息: NumChannels SampleRate ByteRate BlockAlign BitsPerSample
    this->_write_num_channals();
    this->_write_sample_rate();
    this->_write_byte_rate();
    this->_write_block_alian();
    this->_write_bits_per_sample();
}