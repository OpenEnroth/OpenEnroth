#pragma once

#include <string>

#include <al.h> // NOLINT: not a C system header.

#include "Media/AudioTrack.h"

#include "OpenALUpdateThread.h"

class OpenALTrack16 : public IAudioTrack {
 public:
    OpenALTrack16();
    virtual ~OpenALTrack16();

    virtual bool Open(PAudioDataSource data_source) override;
    virtual bool IsValid() override;

    virtual bool Play() override;
    virtual bool Stop() override;
    virtual bool Pause() override;
    virtual bool Resume() override;
    virtual bool SetVolume(float volume) override;
    virtual float GetVolume() override;

 protected:
    void Close();
    void DrainBuffers();
    bool Update();

    PAudioDataSource pDataSource;
    OpenALUpdateThread updater;
    ALenum al_format;
    ALuint al_source;
    ALsizei al_sample_rate;
    size_t uiReservedData;
    size_t uiReservedDataMinimum;
};

PAudioTrack CreateAudioTrack(Blob data);
