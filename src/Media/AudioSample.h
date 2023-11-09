#pragma once

#include <memory>

#include "AudioDataSource.h"

class IAudioSample {
 public:
    IAudioSample() {}
    virtual ~IAudioSample() {}

    virtual bool Open(PAudioDataSource data_source) = 0;
    virtual bool IsValid() = 0;
    virtual bool IsStopped() = 0;

    virtual bool Play(bool loop = false, bool positioned = false) = 0;
    virtual bool Stop() = 0;
    virtual bool Pause() = 0;
    virtual bool Resume() = 0;
    virtual bool SetVolume(float volume) = 0;
    virtual bool SetPosition(float x, float y, float z, float max_dist) = 0;
};
typedef std::shared_ptr<IAudioSample> PAudioSample;
