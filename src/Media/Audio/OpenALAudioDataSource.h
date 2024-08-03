#pragma once

#include <memory>
#include <vector>

#include <al.h> // NOLINT: not a system C header.

#include "Media/AudioDataSource.h"

// TODO(Nik-RE-dev): this middleware class is temporary because Media API is not fully
// ready to properly support current use cases
class OpenALAudioDataSource : public IAudioDataSource {
 public:
    explicit OpenALAudioDataSource(PAudioDataSource baseDataSource):_baseDataSource(baseDataSource) {}
    virtual ~OpenALAudioDataSource() override;

    virtual bool Open() override;
    virtual void Close() override;

    virtual size_t GetSampleRate() override { return _baseDataSource->GetSampleRate(); }
    virtual size_t GetChannelCount() override { return _baseDataSource->GetChannelCount(); }
    virtual Blob GetNextBuffer() override { return _baseDataSource->GetNextBuffer(); }

    virtual float GetDuration() override { return _baseDataSource->GetDuration(); }

    bool linkSource(ALuint al_source);

 protected:
    PAudioDataSource _baseDataSource;
    std::vector<ALuint> _buffers;
};

PAudioDataSource PlatformDataSourceInitialize(PAudioDataSource baseDataSource);
