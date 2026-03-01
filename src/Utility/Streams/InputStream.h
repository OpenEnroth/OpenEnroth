#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <algorithm>

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

        if (size <= bufferRemaining()) {
            memcpy(data, _pos, size);
            _pos += size;
            return size;
        }

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

        if (size <= bufferRemaining()) {
            _pos += size;
            return size;
        }

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

        if (const char *pos = static_cast<const char *>(memchr(_pos, delimiter, std::min(bufferRemaining(), maxSize)))) {
            size_t size = pos - _pos;
            dst->append(_pos, size);
            _pos = pos + 1;
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
     * Initializes the stream from a memory region.
     *
     * @param bufferStart               Pointer to the start of the data to read from.
     * @param bufferEnd                 Pointer past the end of the data to read from.
     * @param displayPath               Display path for error reporting.
     */
    InputStream(const void *bufferStart, const void *bufferEnd, std::string_view displayPath = {});

    /**
     * Re-initializes the stream from a memory region.
     *
     * @param bufferStart               Pointer to the start of the data to read from.
     * @param bufferEnd                 Pointer past the end of the data to read from.
     * @param displayPath               Display path for error reporting.
     */
    void open(const void *bufferStart, const void *bufferEnd, std::string_view displayPath = {});

    /**
     * Re-initializes the stream with an empty buffer. Data will be fetched via `_underflow()` on demand.
     *
     * @param displayPath               Display path for error reporting.
     */
    void open(std::string_view displayPath);

    [[nodiscard]] const char *bufferPos() const { return _pos; }
    [[nodiscard]] const char *bufferEnd() const { return _end; }
    [[nodiscard]] size_t bufferRemaining() const { return static_cast<size_t>(_end - _pos); }

    /**
     * Fetches more data from the underlying source. Override in subclasses that perform I/O.
     *
     * Operates in three modes depending on the arguments:
     * - Read mode (`data != nullptr`): reads up to `size` bytes into `data`, either directly or via internal buffer.
     * - Skip mode (`data == nullptr, size > 0`): skips up to `size` bytes.
     * - Refill mode (`data == nullptr, size == 0`): fills the internal buffer for scanning (used by `readUntil`).
     *
     * In all modes, sets `*bufferStart` and `*bufferEnd` to the new buffer window (or both to `nullptr` if no
     * buffered data remains).
     *
     * @param[out] data                 Buffer to read into, or `nullptr` for skip/refill.
     * @param size                      Number of bytes to read or skip.
     * @param[out] bufferStart          Set to the new buffer read position.
     * @param[out] bufferEnd            Set to the new buffer end.
     * @return                          Number of bytes read into `data` or skipped.
     * @throws Exception                On error.
     */
    virtual size_t _underflow(void *data, size_t size, const void **bufferStart, const void **bufferEnd);

    /**
     * Reads all remaining data from the underlying source, appending to the provided string. Called by `readAll()`
     * after the buffer has been drained. Default returns 0. Override in subclasses that perform I/O.
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
    const char *_pos = nullptr;
    const char *_end = nullptr;
    bool _isOpen = false;
    std::string _displayPath;
};
