#ifndef SRP_PROJECT_CONFIG_H
#define SRP_PROJECT_CONFIG_H

#define SAMPLE_RATE 44100 // 设定采样率

#define FRAME_MILLISECOND 50 // 50ms 采样周期

#define FRAME_SIZE 2205 // 44100 * (50ms / 1000ms) 采样数大小

#define SNORE_INPUT_SIZE 2646000 // 44100 * 60s 鼾声识别输入大小

#define SNORE_PADDING_SIZE 2205 // 鼾声识别数据填充大小

#define SNORE_BUFFER_SIZE 2648205 // 2646000 + 2205 鼾声识别缓存大小

#define SPL_MILLISECOND 200 // 200ms SPL 计算周期

#define SPL_INPUT_SIZE 8820 // 44100 * (50ms / 1000ms) SPL 输入大小

#define ENABLE_LOG // 启用 log

#ifdef ENABLE_LOG
#define ENABLE_LOG_I // 启用 info
#define ENABLE_LOG_D // 启用 debug
#define ENABLE_LOG_W // 启用 warn
#define ENABLE_LOG_E // 启用 error
#define ENABLE_LOG_F // 启用 fatal
#endif

#ifdef ENABLE_LOG
#define BENCHMARK // 启用耗时计算
#endif

#endif