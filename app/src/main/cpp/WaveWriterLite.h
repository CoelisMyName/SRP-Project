/**
 * @file WaveWriterLite.h
 * @author Galactic-Y (1425136616@qq.com)
 * @brief 波形(PCM)数据保存为文件
 * 当前仅支持保存并封装为无压缩的WAVE文件（*.wav）
 * 音频类型支持 多通道 及 多 byte 模式，在创建编码器时设置。
 * 仅对输入流做简单的长度检查（比如16b双通道，则每次输入字节数应当为4的整数倍）
 * 不支持缓冲区功能（即，如果使用双通道同步输入，则不允许一次只输入一个通道）
 * 通过宏 WAVEWRITERLITE_WBYTE_FLIP 可配置是否在接收16bit音频流时翻转高低字节的位置（此选项用于兼容不同大/小端设备）
 * 写入大小信息等情况时，会自动判定当前机器的字节序，不受此宏控制
 * 
 * @version 0.1
 * @date 2022-02-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SRP_PROJECT_WAVEWRITERLITE_H
#define SRP_PROJECT_WAVEWRITERLITE_H

/**
 * @brief 配置是否在接收16bit音频流时翻转高低字节的位置.
 * 此选项用于兼容不同大/小端设备.
 * 写入大小信息等情况时，会自动判定当前机器的字节序，不受此宏控制
 * true:  每 2byte 进行一次翻转，再进行保存;
 * false: 按输入的地址高低顺序直接存放.
 */
#define WAVEWRITERLITE_WBYTE_FLIP true

// 数据类型定义
#include <stdint.h>

class WaveWriterLite
{
public:
    /**
     * @brief Construct a new Wave Writer Lite object.
     * 此阶段会进行文件打开动作
     *
     * @param uri 文件名（含路径）
     * @param sample_rate 音频采样率
     * @param num_channals 音频通道数
     * @param bytes_per_samples 每通道每次采样的字节数
     */
    WaveWriterLite(const char *const uri,
                   const uint32_t sample_rate,
                   const uint16_t num_channals,
                   const uint16_t bytes_per_sample);

    WaveWriterLite(const WaveWriterLite &) = delete;
    WaveWriterLite &operator=(const WaveWriterLite &) = delete;

    ~WaveWriterLite();

    /**
     * @brief 向当前已打开的音频文件写入数据.
     *
     * @param pdata 数组首指针
     * @param length 数组占用的字节数
     * @retval length 成功输出
     * @retval 0 出现错误
     *
     * @note 如果length不为每帧字节数的整数倍，会拒绝写入
     */
    uint32_t write(const int8_t *pdata, const uint32_t length);

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
     * @retval true 编码器正常工作
     * @retval false 出现了异常，使用close()重置编码器
     */
    operator bool();

    /**
     * @brief 指示当前编码器工作是否正常.
     *
     * @retval true 编码器正常工作
     * @retval false 出现了异常，使用close()重置编码器
     */
    bool good();

private:
    /**
     * @brief c标准的文件指针.
     * 为避免 include 污染而使用 void*
     *
     */
    void *_pFile;

    /**
     * @brief 每帧大小. 用于write阶段的大小检查
     *
     */
    uint16_t _frame_size;

    /**
     * @brief 编码器状态
     * 此状态与 .good() 及bool()对应
     */
    bool _good;

    /**
     * @brief 当前机器的字节码状态
     *
     */
    bool _big_endian;

#if WAVEWRITERLITE_WBYTE_FLIP
    /**
     * @brief 每通道每次采样字节数. 仅启用翻转时需要
     * 
     */
    uint16_t _bytes_per_sample;
#endif // WAVEWRITERLITE_WBYTE_FLIP

    /**
     * @brief 文件首固定的riff字段
     *
     */
    const static uint8_t head[22];

    /**
     * @brief data块首的占位符
     *
     */
    const static uint8_t data_head[8];

    /**
     * @brief 检查当前机器的大小端
     *
     * @retval true
     * @retval false
     */
    static bool _check_endian();

    template <typename T>
    friend bool _write_endian(const T data, WaveWriterLite &wwl);
};

#endif // SRP_PROJECT_WAVEWRITERLITE_H
