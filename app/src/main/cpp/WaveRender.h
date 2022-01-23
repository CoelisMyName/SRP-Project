#ifndef SRP_PROJECT_WAVERENDER_H
#define SRP_PROJECT_WAVERENDER_H

#include "GLRender.h"
#include "AudioDataCallback.h"

class WaveRender : public GLRender, public AudioDataCallback {
public:
    virtual ~WaveRender() override = default;

    virtual void onAttach() override;

    virtual void onStart(int64_t timestamp) override;

    virtual void onStop(int64_t timestamp) override;

    virtual void onReceive(int64_t timestamp, int16_t *data, int32_t length) override;

    virtual void onDetach() override;

    virtual void onCreate(int32_t width, int32_t height) override;

    virtual void onDraw() override;

    virtual void onChange(int32_t width, int32_t height) override;

    virtual void onDestroy() override;

private:

};

#endif