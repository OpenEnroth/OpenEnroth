#pragma once

#include <string>

#include "InputStream.h"

class MemoryInputStream: public InputStream {
 public:
    MemoryInputStream();
    MemoryInputStream(const void *data, size_t size, std::string_view displayPath = {});
    virtual ~MemoryInputStream();

    void reset(const void *data, size_t size, std::string_view displayPath = {});

    virtual size_t read(void *data, size_t size) override;
    virtual size_t skip(size_t size) override;
    virtual void close() override;
    [[nodiscard]] std::string displayPath() const override;

    void seek(size_t pos);
    [[nodiscard]] size_t position() const;

 private:
    const char *_begin = nullptr;
    const char *_pos = nullptr;
    const char *_end = nullptr;
    std::string _displayPath;
};
