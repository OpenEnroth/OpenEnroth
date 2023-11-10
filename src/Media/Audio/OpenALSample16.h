#pragma once

#include <AL/al.h>

#include "Media/AudioDataSource.h"
#include "Media/AudioSample.h"

#include "Library/Geometry/Vec.h"

class AudioSample16 : public IAudioSample {
 public:
    AudioSample16() {}
    virtual ~AudioSample16() override;

    virtual bool Open(PAudioDataSource data_source) override;
    virtual bool IsValid() override;
    virtual bool IsStopped() override;

    virtual bool Play(bool loop = false, bool positioned = false) override;
    virtual bool Stop() override;
    virtual bool Pause() override;
    virtual bool Resume() override;
    virtual bool SetVolume(float volume) override;
    virtual bool SetPosition(float x, float y, float z, float max_dist) override;

 protected:
    void Close();
    void defaultSource();

    PAudioDataSource pDataSource = nullptr;
    ALuint al_source = -1;
    Vec3f _position = Vec3f(0.0, 0.0, 0.0);
    float _maxDistance = 0.0;
    float _volume = 0.0;
};

PAudioSample CreateAudioSample();
