#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

#include "Utility/Streams/StreamBuffer.h"

/**
 * Base class for all data input streams.
 *
 * Manages an internal buffer window and implements all read operations on top of it, with inline fast paths
 * for the common case when the requested data is already in the buffer.
 *
 * Data is provided either by setting the buffer directly (for memory-backed streams) or through overriding
 * the `_underflow` virtual method.
 */
class InputStream {
 public:
    using Buffer = StreamBuffer<const char>;

    virtual ~InputStream();

    /**
     * @param[out] data                 Output buffer to write read data into.
     * @param size                      Number of bytes to read.
     * @return                          Number of bytes actually read. A return value that's less than `size` signals
     *                                  end of stream.
     * @throws Exception                On error.
     */
    [[nodiscard]] size_t read(void *data, size_t size) {
        assert(isOpen());
        assert(data || size == 0);

        if (size == 0)
            return 0;

        if (size <= _buffer.remaining())
            return _buffer.read(data, size);

        return underflow(data, size);
    }

    /**
     * Reads the requested amount of data from the stream, or fails with an exception if unable to do so.
     *
     * @param[out] data                 Output buffer to write read data into.
     * @param size                      Number of bytes to read.
     * @throws Exception                On error.
     */
    void readOrFail(void *data, size_t size) {
        size_t bytesRead = read(data, size);
        if (bytesRead != size)
            throwReadError(size, bytesRead);
    }

    /**
     * Reads everything that's in this stream, writing into the provided string.
     *
     * @param[out] dst                  String to write the data into. Previous contents are cleared.
     * @return                          Number of bytes read from the stream.
     * @throws Exception                On error.
     */
    [[nodiscard]] size_t readAll(std::string *dst);

    /**
     * Reads everything that's in this stream.
     *
     * @return                          Data read from the stream, as `std::string`.
     * @throws Exception                On error.
     */
    [[nodiscard]] std::string readAll() {
        std::string result;
        (void) readAll(&result);
        return result;
    }

    /**
     * @param size                      Number of bytes to skip.
     * @return                          Number of bytes actually skipped. A return value that's less than `size` signals
     *                                  end of stream.
     * @throws Exception                On error.
     */
    [[nodiscard]] size_t skip(size_t size) {
        assert(isOpen());

        if (size <= _buffer.remaining())
            return _buffer.skip(size);

        return underflow(nullptr, size);
    }

    /**
     * Same as `readOrFail`, but for skipping bytes.
     *
     * @param size                      Number of bytes to skip.
     * @throws Exception                On error.
     */
    void skipOrFail(size_t size) {
        size_t bytesSkipped = skip(size);
        if (bytesSkipped != size)
            throwSkipError(size, bytesSkipped);
    }

    /**
     * Reads data from the stream until the given delimiter is found, writing into the provided string. The delimiter
     * itself is consumed from the stream but not written to the string.
     *
     * @param delimiter                 Delimiter character to search for.
     * @param[out] dst                  String to write the data into. Previous contents are cleared.
     * @return                          Number of bytes read from the stream, including the delimiter if it was found.
     * @throws Exception                On error.
     */
    [[nodiscard]] size_t readUntil(char delimiter, std::string *dst) {
        assert(isOpen());
        assert(dst);
        dst->clear();

        if (const char *p = static_cast<const char *>(memchr(_buffer.pos(), delimiter, _buffer.remaining()))) {
            size_t bytesRead = _buffer.read(dst, p - _buffer.pos());
            bytesRead += _buffer.skip(1);
            return bytesRead;
        }

        return readUntilSlow(delimiter, dst);
    }

    /**
     * Reads data from the stream until the given delimiter is found and returns it as a string. The delimiter itself
     * is consumed from the stream but not included in the returned string.
     *
     * @param delimiter                 Delimiter character to search for.
     * @return                          Data read from the stream, up to (but not including) the delimiter.
     * @throws Exception                On error.
     */
    [[nodiscard]] std::string readUntil(char delimiter) {
        std::string result;
        (void) readUntil(delimiter, &result);
        return result;
    }

    /**
     * Closes this input stream. Reading from a closed stream will result in undefined behavior.
     *
     * Does nothing if the stream is already closed.
     *
     * @throws Exception                On error.
     */
    void close() {
        if (isOpen())
            _close();
    }

    /**
     * @return                          Whether this stream is open.
     */
    [[nodiscard]] bool isOpen() const { return _isOpen; }

    /**
     * @return                          Current position in the stream, in bytes from the beginning.
     */
    [[nodiscard]] size_t position() const { return _bufferBase + _buffer.used(); }

    /**
     * @return                          Total size of the stream in bytes, or `size_t(-1)` for unsized streams.
     */
    [[nodiscard]] size_t size() const { return _size; }

    /**
     * @return                          Path to the file or resource being read, to be used for debugging and error
     *                                  reporting.
     */
    [[nodiscard]] const std::string &displayPath() const { return _displayPath; }

 protected:
    InputStream() = default;

    /**
     * Initializes the stream with the given buffer.
     *
     * @param buffer                    Initial buffer state.
     * @param size                      Total stream size in bytes, or `size_t(-1)` if unknown.
     * @param displayPath               Display path for error reporting.
     */
    void open(Buffer buffer, size_t size, std::string_view displayPath);

    /**
     * Fetches more data from the underlying source. Override in subclasses that perform I/O.
     *
     * Three modes of operation:
     * - `size == 0`: just refills the buffer without consuming any data.
     * - `data != nullptr`: reads `size` bytes into `data`.
     * - `data == nullptr && size > 0`: skips `size` bytes.
     *
     * In all modes, sets `*buffer` to the new buffer state.
     *
     * @param[out] data                 Buffer to read into, or `nullptr` for skip/refill.
     * @param size                      Number of bytes to read or skip.
     * @param[out] buffer               New buffer state.
     * @return                          Number of bytes read into `data` or skipped.
     * @throws Exception                On error.
     */
    virtual size_t _underflow(void *data, size_t size, Buffer *buffer);

    /**
     * Closes the underlying source, releasing any held resources. Override in subclasses that need cleanup.
     * Derived implementations should call `InputStream::_close()` at the end.
     *
     * @throws Exception                On error.
     */
    virtual void _close();

    [[noreturn]] void throwReadError(size_t requested, size_t actual) const;
    [[noreturn]] void throwSkipError(size_t requested, size_t actual) const;

 private:
    size_t underflow(void *data, size_t size);
    size_t readUntilSlow(char delimiter, std::string *dst);

 private:
    Buffer _buffer;
    size_t _bufferBase = 0;
    size_t _size = static_cast<size_t>(-1);
    bool _isOpen = false;
    std::string _displayPath;
};
