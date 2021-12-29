#ifndef SRP_PROJECT_SNOREAUDIOCALLBACK_H
#define SRP_PROJECT_SNOREAUDIOCALLBACK_H


class SnoreAudioCallback {
public:
    virtual ~SnoreAudioCallback() = default;

    virtual void put(JNIEnv *env, int16_t *data, uint32_t length) = 0;
};

#endif
