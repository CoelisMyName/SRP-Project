/**
 * @file WaveWriter.h
 * @author Galactic-Y (1425136616@qq.com)
 * @brief 波形(PCM)数据保存为文件
 * 当前仅支持保存并封装为无压缩的WAVE文件（*.wav）
 * 音频类型支持 多通道 及 多 byte 模式，在创建编码器时设置。
 * 仅对输入流做简单的长度检查（比如16b双通道，则每次输入字节数应当为4的整数倍）
 * 不支持缓冲区功能（即，如果使用双通道同步输入，则不允许一次只输入一个通道）
 * 可配置是否在接收16bit音频流时翻转高低字节的位置（此选项用于兼容不同大/小端设备）
 * 或使用自动判定当前机器的字节序的模式进行写入（也是文件参数信息的输出方式）
 *
 * @version 0.3
 * @date 2022-02-25
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef SRP_PROJECT_WAVEWRITERLITE_H
#define SRP_PROJECT_WAVEWRITERLITE_H

// 数据类型定义
#include <stdint.h>

class WaveWriter
{
public:

    enum class flip_t{
        AUTO,
        ALWAYS,
        NEVER
    };

    /**
     * @brief Construct a new Wave Writer Lite object.
     * 此阶段会进行文件打开动作
     *
     * @param uri 文件名（含路径）
     * @param sample_rate 音频采样率
     * @param num_channals 音频通道数
     * @param bytes_per_samples 每通道每次采样的字节数
     */
    WaveWriter(const char *const uri,
                   const uint32_t sample_rate,
                   const uint16_t num_channals,
                   const uint16_t bytes_per_sample);

    WaveWriter(const WaveWriter &) = delete;
    WaveWriter &operator=(const WaveWriter &) = delete;

    ~WaveWriter();

    /**
     * @brief 向当前已打开的音频文件写入数据.
     *
     * @param pdata 数组首指针
     * @param length 数组占用的字节数
     * @param reverse 是否反转高低字节顺序
     * @retval length 成功输出
     * @retval 0 出现错误
     *
     * @note 如果length不为每帧字节数的整数倍，会拒绝写入
     */
    uint32_t write(const int8_t *pdata, const size_t length, flip_t reverse = flip_t::AUTO);

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
    void *_m_pfile;

    /**
     * @brief 每帧大小. 用于write阶段的大小检查
     *
     */
    uint16_t _m_frame_size;

    /**
     * @brief 编码器状态
     * 此状态与 .good() 及bool()对应
     */
    bool _m_good;

    /**
     * @brief 当前机器的字节码状态
     *
     */
    bool _m_big_endian;

    /**
     * @brief 每通道每次采样字节数
     *
     */
    uint16_t _m_bytes_per_sample;

    /**
     * @brief 文件首固定的riff字段
     *
     */
    const static uint8_t _m_head[22];

    /**
     * @brief data块首的占位符
     *
     */
    const static uint8_t _m_data_head[8];

    /**
     * @brief 检查当前机器的大小端
     *
     * @retval true 当前机器为大端模式
     * @retval false 当前机器为小端模式
     */
    static bool _check_endian();

    template <typename T>
    friend bool _write_endian(const T data, WaveWriter &wwl);

    /**
     * @brief 更新文件中的大小信息。只由close与write调用
     * 
     * @return true 更新成功
     * @return false 更新失败（包括文件未打开的情况）
     */
    bool _update_size();
};

#endif // SRP_PROJECT_WAVEWRITERLITE_H
