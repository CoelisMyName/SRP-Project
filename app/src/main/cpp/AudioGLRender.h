#ifndef SRP_PROJECT_AUDIOGLRENDER_H
#define SRP_PROJECT_AUDIOGLRENDER_H

#include "GLRender.h"
#include "AudioDataCallback.h"

class AudioGLRender : public GLRender, public AudioDataCallback {
public:
    virtual ~AudioGLRender() = default;
};

#endif