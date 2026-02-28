#pragma once

#include <memory>

#include "AudioBaseDataSource.h"
#include "FFmpegBlobIoContext.h"

class AudioBufferDataSource : public AudioBaseDataSource {
 public:
    explicit AudioBufferDataSource(Blob buffer);
    virtual ~AudioBufferDataSource() = default;

    virtual bool Open() override;

 protected:
    FFmpegBlobIoContext _ioContext;
};

PAudioDataSource CreateAudioBufferDataSource(Blob buffer);
