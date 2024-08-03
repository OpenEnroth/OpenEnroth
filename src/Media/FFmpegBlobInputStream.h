#pragma once

#include "Utility/Memory/Blob.h"
#include "Utility/Streams/BlobInputStream.h"

struct AVIOContext;

class FFmpegBlobInputStream : public BlobInputStream {
 public:
    FFmpegBlobInputStream() = default;
    explicit FFmpegBlobInputStream(Blob blob);
    virtual ~FFmpegBlobInputStream();

    void open(Blob blob);

    virtual void close() override;

    AVIOContext *ioContext() const {
        return _ctx;
    }

 private:
    void closeInternal();

 private:
    AVIOContext *_ctx = nullptr;
};
