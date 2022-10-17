#pragma once

#include "InputStream.h"

class MemoryInputStream: public InputStream {
 public:
    MemoryInputStream();
    MemoryInputStream(const void *data, size_t size);
    virtual ~MemoryInputStream();

    void Reset(const void *data, size_t size);

    virtual size_t Read(void *data, size_t size) override;
    virtual size_t Skip(size_t size) override;

 private:
    const char *pos_ = nullptr;
    const char *end_ = nullptr;
};
