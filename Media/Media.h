#pragma once

#include <memory>
#include <string>

#include "src/tools/MemBuffer.h"

class IAudioDataSource {
 public:
    IAudioDataSource() {}
    virtual ~IAudioDataSource() {}

    virtual bool Open() = 0;
    virtual void Close() = 0;

    virtual size_t GetSampleRate() = 0;
    virtual size_t GetChannelCount() = 0;
    virtual PMemBuffer GetNextBuffer() = 0;
};
typedef std::shared_ptr<IAudioDataSource> PAudioDataSource;

PAudioDataSource CreateAudioFileDataSource(const std::string &file_name);
PAudioDataSource CreateAudioBufferDataSource(PMemBuffer buffer);

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

PAudioTrack CreateAudioTrack(const std::string &file_path);

class IAudioSample {
 public:
    IAudioSample() {}
    virtual ~IAudioSample() {}

    virtual bool Open(PAudioDataSource data_source) = 0;
    virtual bool IsValid() = 0;

    virtual bool Play(bool loop = false, bool positioned = false) = 0;
    virtual bool Stop() = 0;
    virtual bool SetVolume(float volume) = 0;
    virtual bool SetPosition(float x, float y, float z, float max_dist) = 0;
};
typedef std::shared_ptr<IAudioSample> PAudioSample;

PAudioSample CreateAudioSample(PMemBuffer buffer);

class IVideoDataSource {
 public:
    IVideoDataSource() {}
    virtual ~IVideoDataSource() {}

    virtual bool Open() = 0;
    virtual void Close() = 0;

    virtual size_t GetFrameRate() = 0;
    virtual size_t GetWidth() = 0;
    virtual size_t GetHeight() = 0;
    virtual PMemBuffer GetNextFrame() = 0;
};
typedef std::shared_ptr<IAudioDataSource> PAudioDataSource;

class IMovie {
 public:
    IMovie() {}
    virtual ~IMovie() {}

    virtual unsigned int GetWidth() const = 0;
    virtual unsigned int GetHeight() const = 0;

    virtual bool Play(bool loop = false) = 0;
    virtual bool Stop() = 0;
    virtual bool IsPlaying() const = 0;
    virtual PMemBuffer GetFrame() = 0;
    virtual std::string GetFormat() = 0;
    virtual void PlayBink() = 0;
};
typedef std::shared_ptr<IMovie> PMovie;
