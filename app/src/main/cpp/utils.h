#ifndef SRP_PROJECT_UTILS_H
#define SRP_PROJECT_UTILS_H

#include <sox.h>

void initialSox();

void quitSox();

void freeEffects(sox_effect_t *effects[], int count);

void generateNoiseProfile(void *data, size_t size, double start, double duration, char *filename);

size_t reduceNoise(void *srt, size_t srt_sz, void *dest, size_t dest_sz, char *filename,
                   double coefficient);

#endif
