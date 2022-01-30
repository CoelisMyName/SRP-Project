/**
 * @file WaveWriter.cpp
 * @author Galactic-Y (1425136616@qq.com)
 * @brief 波形(PCM)数据保存为文件的简单实现
 * @version 0.1
 * @date 2022-01-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "WaveWriterLite.h"
// 标准c接口文件操作
#include <cstdio>

WaveWriterLite::WaveWriterLite(const char *const uri,
                               const uint32_t sample_rate,
                               const uint16_t num_channals,
                               const uint16_t bytes_per_samples)
{
}

WaveWriterLite::~WaveWriterLite()
{
}
