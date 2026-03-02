#pragma once

#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "OutputStream.h"

/**
 * Output stream that writes to a file.
 */
class FileOutputStream : public OutputStream {
    using base_type = OutputStream;

 public:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 1024 * 1024;

    FileOutputStream() = default;

    /**
     * @param path                      Path to the file to open.
     * @param bufferSize                Size of the internal write buffer.
     * @throws Exception                On error.
     */
    explicit FileOutputStream(std::string_view path, size_t bufferSize = DEFAULT_BUFFER_SIZE);
    ~FileOutputStream();

    /**
     * Opens a file for writing.
     *
     * @param path                      Path to the file to open.
     * @param bufferSize                Size of the internal write buffer.
     * @throws Exception                On error.
     */
    void open(std::string_view path, size_t bufferSize = DEFAULT_BUFFER_SIZE);

    /**
     * @return                          The underlying file handle.
     */
    [[nodiscard]] FILE *handle() {
        return _file;
    }

 private:
    virtual void _overflow(const void *data, size_t size, Buffer *buffer) override;
    virtual void _flush(Buffer *buffer) override;
    virtual void _close() override;

    void writeBuffer(const Buffer &buffer);
    void closeInternal(bool canThrow);

 private:
    std::string _path;
    FILE *_file = nullptr;
    std::unique_ptr<char[]> _buf;
    size_t _bufSize = 0;
};
