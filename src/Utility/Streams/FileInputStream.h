#pragma once

#include <cstdio>
#include <memory>
#include <string_view>

#include "InputStream.h"

// TODO(captainurist): just use raw file io, not FILE*

/**
 * Input stream that reads from a file.
 */
class FileInputStream : public InputStream {
    using base_type = InputStream;

 public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 1024 * 1024;

    FileInputStream() = default;

    /**
     * @param path                      Path to the file to open.
     * @param bufferSize                Size of the internal read buffer.
     * @throws Exception                On error.
     */
    explicit FileInputStream(std::string_view path, size_t bufferSize = DEFAULT_BUFFER_SIZE);
    virtual ~FileInputStream();

    /**
     * Opens a file for reading.
     *
     * @param path                      Path to the file to open.
     * @param bufferSize                Size of the internal read buffer.
     * @throws Exception                On error.
     */
    void open(std::string_view path, size_t bufferSize = DEFAULT_BUFFER_SIZE);

 private:
    virtual size_t _underflow(void *data, size_t size, Buffer *buffer) override;
    virtual void _close() override;

    void closeInternal(bool canThrow);

 private:
    FILE *_file = nullptr;
    std::unique_ptr<char[]> _buf;
    size_t _bufSize = 0;
};
