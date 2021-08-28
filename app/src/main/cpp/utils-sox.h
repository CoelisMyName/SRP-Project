//
// Created by Carlyle on 2021/8/28.
//

#ifndef SRP_PROJECT_UTILS_SOX_H
#define SRP_PROJECT_UTILS_SOX_H

void generateNoiseProfile(void *data, size_t size, double start, double duration, const char *filename);

size_t reduceNoise(void *srt, size_t srt_sz, void *dest, size_t dest_sz, const char *filename,
                   double coefficient);

#endif //SRP_PROJECT_UTILS_SOX_H
