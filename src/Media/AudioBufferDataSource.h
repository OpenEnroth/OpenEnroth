#pragma once

#include <memory>

#include "AudioBaseDataSource.h"
#include "FFmpegBlobInputStream.h"

struct AVIOContext;

class AudioBufferDataSource : public AudioBaseDataSource {
 public:
    explicit AudioBufferDataSource(Blob buffer);
    virtual ~AudioBufferDataSource() = default;

    virtual bool Open() override;

 protected:
    FFmpegBlobInputStream stream;
};

PAudioDataSource CreateAudioBufferDataSource(Blob buffer);
