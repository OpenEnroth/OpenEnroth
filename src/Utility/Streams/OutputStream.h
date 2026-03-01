#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>

#include "Utility/Memory/Blob.h"

/**
 * Base class for all data output streams.
 *
 * Manages an internal write buffer and implements all write operations on top of it, with an inline fast path
 * for the common case when the data fits in the buffer.
 *
 * Subclasses provide the actual I/O by overriding `_overflow`, `_flush`, and `_close`.
 */
class OutputStream {
 public:
    virtual ~OutputStream();

    /**
     * Writes provided data into the output stream.
     *
     * @param data                      Pointer to the data to write.
     * @param size                      Data size.
     * @throws Exception                On error.
     */
    void write(const void *data, size_t size) {
        assert(isOpen());

        if (size <= bufferRemaining()) {
            memcpy(_pos, data, size);
            _pos += size;
            return;
        }

        writeSlow(data, size);
    }

    /**
     * Writes provided string into the output stream.
     *
     * @param s                         String to write.
     * @throws Exception                On error.
     */
    void write(std::string_view s) {
        write(s.data(), s.size());
    }

    /**
     * Writes provided `Blob` into the output stream.
     *
     * @param blob                      `Blob` to write.
     * @throws Exception                On error.
     */
    void write(const Blob &blob) {
        write(blob.data(), blob.size());
    }

    /**
     * Flushes this output stream, writing any buffered data to the underlying target.
     *
     * @throws Exception                On error.
     */
    void flush() {
        assert(isOpen());
        _flush();
    }

    /**
     * Closes this output stream. Writing into or flushing a closed stream will result in undefined behavior.
     *
     * Does nothing if the stream is already closed.
     *
     * @throws Exception                On error.
     */
    void close() {
        if (!isOpen())
            return;
        _close();
    }

    /**
     * @return                          Whether this stream is open.
     */
    [[nodiscard]] bool isOpen() const { return _isOpen; }

    /**
     * @return                          Path to the file or resource being written, to be used for debugging and error
     *                                  reporting.
     */
    [[nodiscard]] const std::string &displayPath() const { return _displayPath; }

 protected:
    OutputStream() = default;

    /**
     * Re-initializes the stream. The first write will trigger `_overflow()` to obtain the initial buffer.
     *
     * @param displayPath               Display path for error reporting.
     */
    void open(std::string_view displayPath = {});

    /**
     * Re-initializes the stream with a pre-allocated buffer.
     *
     * @param bufferStart               Pointer to the start of the write buffer.
     * @param bufferEnd                 Pointer past the end of the write buffer.
     * @param displayPath               Display path for error reporting.
     */
    void open(void *bufferStart, void *bufferEnd, std::string_view displayPath = {});

    [[nodiscard]] char *bufferPos() const { return _pos; }
    [[nodiscard]] char *bufferEnd() const { return _end; }
    [[nodiscard]] size_t bufferRemaining() const { return static_cast<size_t>(_end - _pos); }

    /**
     * Called when the write buffer is full and there is more data to write. Flushes the full buffer, writes or
     * buffers the provided data, and provides a new writable buffer via `bufferStart` and `bufferEnd`. Override in subclasses.
     *
     * @param data                      Pointer to the overflow data to write.
     * @param size                      Size of the overflow data.
     * @param[out] bufferStart          Set to the new buffer write position.
     * @param[out] bufferEnd            Set to the new buffer end.
     * @throws Exception                On error.
     */
    virtual void _overflow(const void *data, size_t size, void **bufferStart, void **bufferEnd) = 0;

    /**
     * Flushes the current buffer and propagates the flush downstream. Override in subclasses.
     *
     * @throws Exception                On error.
     */
    virtual void _flush() = 0;

    /**
     * Flushes the current buffer and releases any held resources. Override in subclasses.
     * Derived implementations should call `OutputStream::_close()` at the end.
     *
     * @throws Exception                On error.
     */
    virtual void _close() = 0;

 private:
    void closeInternal();
    void writeSlow(const void *data, size_t size);

 private:
    char *_pos = nullptr;
    char *_end = nullptr;
    bool _isOpen = false;
    std::string _displayPath;
};
