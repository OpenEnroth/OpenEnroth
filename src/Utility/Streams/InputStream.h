#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <algorithm>

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

        return readSlow(data, size);
    }

    /**
     * Reads the requested amount of data from the stream, or fails with an exception if unable to do so.
     *
     * @param[out] data                 Output buffer to write read data into.
     * @param size                      Number of bytes to read.
     * @throws Exception                On error.
     */
    void readOrFail(void *data, size_t size) {
        size_t bytes = read(data, size);
        if (bytes != size)
            throwReadError(size, bytes);
    }

    /**
     * Reads everything that's in this stream, up to `maxSize` bytes, writing into the provided string.
     *
     * @param[out] dst                  String to write the data into. Previous contents are cleared.
     * @param maxSize                   Maximal number of bytes to read from the stream.
     * @return                          Number of bytes read from the stream.
     * @throws Exception                On error.
     */
    [[nodiscard]] size_t readAll(std::string *dst, size_t maxSize = -1);

    /**
     * Reads everything that's in this stream, up to `maxSize` bytes.
     *
     * @param maxSize                   Maximal number of bytes to read from the stream.
     * @return                          Data read from the stream, as `std::string`.
     * @throws Exception                On error.
     */
    [[nodiscard]] std::string readAll(size_t maxSize = -1) {
        std::string result;
        (void) readAll(&result, maxSize);
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

        return skipSlow(size);
    }

    /**
     * Same as `readOrFail`, but for skipping bytes.
     *
     * @param size                      Number of bytes to skip.
     * @throws Exception                On error.
     */
    void skipOrFail(size_t size) {
        size_t bytes = skip(size);
        if (bytes != size)
            throwSkipError(size, bytes);
    }

    /**
     * Reads data from the stream until the given delimiter is found, writing into the provided string. The delimiter
     * itself is consumed from the stream but not written to the string.
     *
     * @param delimiter                 Delimiter character to search for.
     * @param[out] dst                  String to write the data into. Previous contents are cleared.
     * @param maxSize                   Maximal number of bytes to read from the stream.
     * @return                          Number of bytes read from the stream, including the delimiter if it was found.
     * @throws Exception                On error.
     */
    [[nodiscard]] size_t readUntil(char delimiter, std::string *dst, size_t maxSize = -1) {
        assert(isOpen());
        assert(dst);
        dst->clear();

        if (const char *p = static_cast<const char *>(memchr(_buffer.pos(), delimiter, std::min(_buffer.remaining(), maxSize)))) {
            size_t size = p - _buffer.pos();
            _buffer.read(dst, size);
            _buffer.skip(1);
            return size + 1;
        }

        return readUntilSlow(delimiter, dst, maxSize);
    }

    /**
     * Reads data from the stream until the given delimiter is found and returns it as a string. The delimiter itself
     * is consumed from the stream but not included in the returned string.
     *
     * @param delimiter                 Delimiter character to search for.
     * @param maxSize                   Maximal number of bytes to read from the stream.
     * @return                          Data read from the stream, up to (but not including) the delimiter.
     * @throws Exception                On error.
     */
    [[nodiscard]] std::string readUntil(char delimiter, size_t maxSize = -1) {
        std::string result;
        (void) readUntil(delimiter, &result, maxSize);
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
        if (!isOpen())
            return;
        _close();
    }

    /**
     * @return                          Whether this stream is open.
     */
    [[nodiscard]] bool isOpen() const { return _isOpen; }

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
     * @param displayPath               Display path for error reporting.
     */
    explicit InputStream(Buffer buffer, std::string_view displayPath = {});

    /**
     * Re-initializes the stream with the given buffer.
     *
     * @param buffer                    Initial buffer state.
     * @param displayPath               Display path for error reporting.
     */
    void open(Buffer buffer, std::string_view displayPath = {});

    /**
     * @return                          Current buffer state.
     */
    [[nodiscard]] const Buffer &buffer() const { return _buffer; }

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
     * @param[in,out] buffer            Current buffer state on input (always empty). Set to the new buffer state
     *                                  on output.
     * @return                          Number of bytes read into `data` or skipped.
     * @throws Exception                On error.
     */
    virtual size_t _underflow(void *data, size_t size, Buffer *buffer);

    /**
     * Reads remaining data from the underlying source, appending to the provided string. Called by `readAll()`
     * after consuming up to `maxSize` bytes from the buffer. Default returns 0. Override in subclasses that
     * perform I/O.
     *
     * @param[out] dst                  String to append the data to.
     * @param maxSize                   Maximal number of bytes to read.
     * @return                          Number of bytes read from the source.
     * @throws Exception                On error.
     */
    virtual size_t _readAll(std::string *dst, size_t maxSize);

    /**
     * Closes the underlying source, releasing any held resources. Override in subclasses that need cleanup.
     * Derived implementations should call `InputStream::_close()` at the end.
     *
     * @throws Exception                On error.
     */
    virtual void _close();

 private:
    void closeInternal();
    size_t readSlow(void *data, size_t size);
    size_t skipSlow(size_t size);
    size_t readUntilSlow(char delimiter, std::string *dst, size_t maxSize);
    [[noreturn]] static void throwReadError(size_t requested, size_t actual);
    [[noreturn]] static void throwSkipError(size_t requested, size_t actual);

 private:
    Buffer _buffer;
    bool _isOpen = false;
    std::string _displayPath;
};
