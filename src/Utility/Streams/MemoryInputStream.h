#pragma once

#include <stddef.h>

#include "InputStream.h"

class MemoryInputStream: public InputStream {
 public:
    MemoryInputStream();
    MemoryInputStream(const void *data, size_t size);
    virtual ~MemoryInputStream();

    void reset(const void *data, size_t size);

    virtual size_t read(void *data, size_t size) override;
    virtual size_t skip(size_t size) override;
    virtual void close() override;

 private:
    const char *_pos = nullptr;
    const char *_end = nullptr;
};
