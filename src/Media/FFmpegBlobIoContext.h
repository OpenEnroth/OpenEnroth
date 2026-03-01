#pragma once

#include "Utility/Memory/Blob.h"

struct AVIOContext;

class FFmpegBlobIoContext {
 public:
    FFmpegBlobIoContext() = default;
    explicit FFmpegBlobIoContext(Blob blob);
    ~FFmpegBlobIoContext();

    void reset(Blob blob);

    AVIOContext *avioContext() const {
        return _ctx;
    }

    const Blob &blob() const {
        return _blob;
    }

 private:
    friend int ffRead(void *opaque, uint8_t *buf, int size);
    friend int64_t ffSeek(void *opaque, int64_t offset, int whence);

    void createAvioContext();
    void destroyAvioContext();

    Blob _blob;
    size_t _pos = 0;
    AVIOContext *_ctx = nullptr;
};
