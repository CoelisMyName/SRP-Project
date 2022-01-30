/**
 * @file WaveWriter.h
 * @author Galactic-Y (1425136616@qq.com)
 * @brief 波形(PCM)数据保存为文件
 * 当前仅支持保存并封装为无压缩的WAVE文件（*.wav）
 * 音频类型支持 单/双通道 及 8/16b 组合共四种模式，在创建编码器时设置。
 * 通过宏 WAVEWRITER_WBYTE_FLIP 可配置是否在接收16bit音频流时翻转高低字节的位置（此选项用于兼容不同大/小端设备）
 * @version 0.1
 * @date 2022-01-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SRP_PROJECT_WAVEWRITER_H
#define SRP_PROJECT_WAVEWRITER_H

/**
 * @brief 配置是否在接收16bit音频流、写入大小信息时翻转高低字节的位置.
 * 此选项用于兼容不同大/小端设备.
 * true:  每 2byte 进行一次翻转，再进行保存;
 * false: 按输入的地址高低顺序直接存放.
 */
#define WAVEWRITER_WBYTE_FLIP true

// 数据类型定义
#include <stdint.h>

class WaveWriter
{
public:
    /**
     * @brief 音频流的类型。单/双通道 及 8/16b 组合共四种模式
     *
     */
    enum class FORMAT : uint8_t
    {
        CH1_BIT8,  // 8bit  单声道
        CH2_BIT8,  // 8bit  立体声
        CH1_BIT16, // 16bit 单声道
        CH2_BIT16  // 16bit 立体声
    };

    /**
     * @brief 当前流输出的通道
     *
     */
    enum class CHANNAL : uint8_t
    {
        CH0,  // 通道0（默认通道）
        CH1,  // 通道1（仅立体声模式下有效）
        MULTI // 当前数组中通道0与1交错存放（仅立体声模式下有效）
    };

    /**
     * @brief Construct a new Wave Writer object
     *
     * @param sample_rate 音频采样率
     * @param format 音频通道格式
     */
    WaveWriter(uint32_t sample_rate, FORMAT format = FORMAT::CH1_BIT16);
    WaveWriter(const WaveWriter &) = delete;
    ~WaveWriter();

    /**
     * @brief 获取当前编码器的采样率参数
     *
     * @return uint32_t 当前编码器的采样率
     */
    uint32_t get_sample_rate() const;

    /**
     * @brief 改变当前编码器的采样率.
     * 如果有文件正在输出，会拒绝修改（修改失败）
     *
     * @param sample_rate 新的采样率
     * @retval true  设置成功
     * @retval false 设置失败
     */
    bool set_sample_rate(uint32_t sample_rate);

    /**
     * @brief 获取当前编码器接受的码流（通道数及采样深度）
     *
     * @return FORMAT 当前编码器接受的码流格式
     */
    FORMAT get_format() const;
    /**
     * @brief 设置当前编码器接受的码流格式
     * 如果已打开一个文件且未关闭，则设置失败.
     *
     * @param sample_rate 音频流采样率
     * @param format 音频流格式
     * @retval true  修改成功
     * @retval false 修改失败
     */
    bool set_format(uint32_t sample_rate, FORMAT format = FORMAT::CH1_BIT16);
    /**
     * @brief 设置当前编码器接受的码流格式
     * 如果已打开一个文件且未关闭，则设置失败.
     *
     * @param ref 用于参照的编码器，将从此编码器读取参数
     * @retval true  修改成功
     * @retval false 修改失败
     */
    bool set_format(const WaveWriter &ref);
    /**
     * @brief 打开文件并准备输出音频流（覆盖原文件）.
     * 如果已打开一个文件且未关闭，则会打开失败.
     *
     * @param uri 文件路径（含文件名）
     * @retval true  打开成功
     * @retval false 打开失败
     */
    bool open(const char *const uri);
    /**
    /**
     * @brief 打开文件并准备输出音频流（覆盖原文件）.
     * 使用参数所指定的编码格式（相当于打开前执行set_format()）.
     * 如果已打开一个文件且未关闭，则会打开失败.
     *
     * @param uri 文件路径（含文件名）
     * @param sample_rate 音频流采样率
     * @param format 音频流格式
     * @retval true  打开成功
     * @retval false 打开失败
     */
    bool open(const char *const uri, uint32_t sample_rate, FORMAT format);
    /**
     * @brief 向当前已打开的音频文件的指定声道写入数据.
     *
     * @param pdata 数组首指针
     * @param length 数组占用的字节数
     * @param ch 输出的目标通道（单通道文件忽视此选项）
     * @return uint32_t 成功输出的字节数。输出失败返回0.
     *
     * @note 如果通过单通道输出导致两个通道数据进度不一致，将触发一次 sync()
     */
    uint32_t write(const int8_t *pdata, const uint32_t length, CHANNAL ch = CHANNAL::CH0);

    /**
     * @brief 向当前已打开的音频文件的指定声道写入数据.
     * 对于深度为 16bit 的数据推荐使用此方法进行输出
     *
     * @param pdata 数组首指针
     * @param length 数组占用的字节数
     * @param ch 输出的目标通道（单通道文件忽视此选项）
     * @return uint32_t 成功输出的字节数。输出失败返回0.
     *
     * @note 如果通过单通道输出导致两个通道数据进度不一致，将触发一次 sync()
     */
    uint32_t writew(const int16_t *pdata, const uint32_t length, CHANNAL ch = CHANNAL::CH0);

    /**
     * @brief （立体声下使两个通道的进度对齐，并）将缓冲区内所有内容写入文件.
     *
     * @param use_zero 指示是否使用0对另一通道进行填充. 为true使用0，为false使用另一通道数据. 单通道音频文件忽略此选项.
     * @retval true 写入成功（含缓冲区为空）
     * @retval false 写入失败
     */
    bool flush(bool use_zero = true);

    /**
     * @brief 关闭当前文件并写入文件尾
     *
     * @retval true 关闭成功（含未打开文件）
     * @retval false 关闭失败
     */
    bool close();

    /**
     * @brief 指示当前编码器工作是否正常.
     * 
     * @retval true 编码器正常工作或未在工作
     * @retval false 出现了异常，使用close()重置编码器
     */
    operator bool(){
        return !this->_bad;
    }

private:
    // 码率
    uint32_t _sample_rate;
    // 输出序列格式
    FORMAT _origin_fmt;
    bool _dual_ch;
    bool _dual_byte;
    // 文件指针, 为避免include 污染，这里用void*存放（而非FILE*）
    void *_pfile;

    // 数据大小
    uint32_t _data_size;
    // 当前文件是否出现过bad
    bool _bad;

    /* 缓冲区相关选项 */
    // 如果宏 WAVEWRITER_WBYTE_FLIP 开启，则缓冲区内的数据已经经过翻转

    // 输出缓存. 每次只有一个通道占用此缓存. 此成员用于delete[]
    int8_t *_pbuff;
    // 缓冲区内下一个输出的数据指针（刚创建时与_pbuff相等）
    int8_t *_buff_pdata;
    // 缓冲区尾部指针+1，用于判定输出到缓冲区尾（_buff_pdata == _buff_end 时结束循环）
    int8_t *_buff_end;
    // 当前缓冲区的占用情况. 将 MULYI 用于表示没有通道占用缓冲区
    CHANNAL _which_ch;
    // 缓冲区内剩余字节数的大小（用于合并缓冲区）
    uint32_t _left_size;

    const static uint8_t head[44];

    /**
     * @brief 更新RIFF块中的size信息
     * 位于文件第04-07个字节处。
     */
    void _write_riff_size();

    /**
     * @brief 更新data块中的size信息
     * 位于文件第0x4A-0x4D个字节处。
     */
    void _write_data_size();

    /**
     * @brief 更新fmt块中的声道数信息
     * 位于文件第0x16-0x17个字节处。
     */
    void _write_num_channals();

    /**
     * @brief 更新fmt块中的采样率信息
     * 位于文件第0x18-0x1B个字节处。
     */
    void _write_sample_rate();

    /**
     * @brief 更新fmt块中的字节速率信息
     * 位于文件第0x1C-0x1F个字节处。
     */
    void _write_byte_rate();

    /**
     * @brief 更新fmt块中的字节对齐信息（每帧字节数）
     * 位于文件第0x20-0x21个字节处。
     */
    void _write_block_alian();

    /**
     * @brief 更新fmt块中的采样深度信息（按位）
     * 位于文件第0x22-0x23个字节处。
     */
    void _write_bits_per_sample();

    /**
     * @brief 初始化文件头部.  
     * 将文件填充至data块的内容区开始前. 
     * 此时格式信息未写入，应当调用各个_write
     */
    void _file_head_init();
};

#endif // SRP_PROJECT_WAVEWRITER_H
