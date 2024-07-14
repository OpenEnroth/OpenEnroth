#pragma once

#include <queue>
#include <deque>
#include <memory>

#include "AudioDataSource.h"

struct AVFormatContext;
struct AVCodecContext;
struct SwrContext;

class AudioBaseDataSource : public IAudioDataSource {
 public:
    AudioBaseDataSource();
    virtual ~AudioBaseDataSource() { Close(); }

    virtual bool Open() override;
    virtual void Close() override;

    virtual size_t GetSampleRate() override;
    virtual size_t GetChannelCount() override;
    virtual Blob GetNextBuffer() override;

    virtual float GetDuration() override;

 protected:
    AVFormatContext *pFormatContext;
    int iStreamIndex;
    AVCodecContext *pCodecContext;
    SwrContext *pConverter;
    bool bOpened;
    std::queue<Blob> queue;

    float _savedDuration;
};
