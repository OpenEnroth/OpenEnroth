#pragma once

#include <memory>

#include "AudioBaseDataSource.h"

struct AVIOContext;

class AudioBufferDataSource : public AudioBaseDataSource {
 public:
    explicit AudioBufferDataSource(Blob buffer);
    virtual ~AudioBufferDataSource() {}

    virtual bool Open() override;

 protected:
    Blob buffer;
    const uint8_t *buf_pos;
    const uint8_t *buf_end;
    uint8_t *avio_ctx_buffer;
    size_t avio_ctx_buffer_size;
    AVIOContext *avio_ctx;

    static int read_packet(void *opaque, uint8_t *buf, int buf_size);
    int ReadPacket(uint8_t *buf, int buf_size);

    static int64_t seek(void *opaque, int64_t offset, int whence);
    int64_t Seek(void *opaque, int64_t offset, int whence);
};

PAudioDataSource CreateAudioBufferDataSource(Blob buffer);
