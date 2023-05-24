#pragma once

#include "Utility/Memory/Blob.h"

#include "InputStream.h"

class BlobInputStream : public InputStream {
 public:
    BlobInputStream() = default;
    explicit BlobInputStream(const Blob &blob);

    virtual size_t read(void *data, size_t size) override;
    virtual size_t skip(size_t size) override;
    virtual void close() override;
    Blob tail() const;

 private:
    const Blob *_blob = nullptr;
    const char *_pos = nullptr;
    const char *_end = nullptr;
};
