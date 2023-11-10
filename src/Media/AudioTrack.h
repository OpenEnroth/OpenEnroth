#pragma once

#include <memory>

#include "AudioDataSource.h"

class IAudioTrack {
 public:
    IAudioTrack() {}
    virtual ~IAudioTrack() {}

    virtual bool Open(PAudioDataSource data_source) = 0;
    virtual bool IsValid() = 0;

    virtual bool Play() = 0;
    virtual bool Stop() = 0;
    virtual bool Pause() = 0;
    virtual bool Resume() = 0;
    virtual bool SetVolume(float volume) = 0;
    virtual float GetVolume() = 0;
};
typedef std::shared_ptr<IAudioTrack> PAudioTrack;

