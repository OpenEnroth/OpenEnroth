#pragma once

#include <mutex>
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
    ALenum al_format = 0;
    ALuint al_source = 0;
    ALsizei al_sample_rate = 0;
    size_t uiReservedData = 0;
    size_t uiReservedDataMinimum = 0;
    std::mutex _mutex;  // Protects pDataSource access.
};

PAudioTrack CreateAudioTrack(Blob data);
