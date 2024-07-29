#pragma once

#include <string>

#include "Utility/Types.h"

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

    void seek(ssize_t pos);
    [[nodiscard]] ssize_t position() const;
    [[nodiscard]] ssize_t size() const;

 private:
    const char *_begin = nullptr;
    const char *_pos = nullptr;
    const char *_end = nullptr;
    std::string _displayPath;
};
