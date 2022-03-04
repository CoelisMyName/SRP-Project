#ifndef SRP_PROJECT_RENDERFACTORY_H
#define SRP_PROJECT_RENDERFACTORY_H

#include <string.h>
#include "WaveRender.h"
#include "DefaultRender.h"

static const char *const WAVE_RENDER = "wave";
static const char *const DEFAULT_RENDER = "default";

AudioGLRender *newRender(const char *s) {
    AudioGLRender *render = nullptr;
    if (strcmp(s, WAVE_RENDER) == 0) {
        render = new WaveRender();
    }
    if (strcmp(s, DEFAULT_RENDER) == 0) {
        render = new DefaultRender();
    }
    return render;
}

void deleteRender(AudioGLRender *render) {
    delete render;
}

#endif