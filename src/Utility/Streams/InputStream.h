#pragma once

#include <cstddef>

/**
 * Abstract base class for all data input streams.
 *
 * Compared to `std::istream` it is:
 * - Non-buffered.
 * - Uses exceptions for error handling.
 * - Has a very simple and intuitive interface.
 */
class InputStream {
 public:
    virtual ~InputStream() {}

    /**
     * @param data                      Output buffer to write read data into.
     * @param size                      Number of bytes to read.
     * @return                          Number of bytes actually read. A return value that's less than `size` signals
     *                                  end of stream.
     * @throws Exception                On error.
     */
    [[nodiscard]] virtual size_t read(void *data, size_t size) = 0;

    /**
     * Reads the requested amount of data from the stream, or fails with an exception if unable to do so.
     *
     * @param data                      Output buffer to write read data into.
     * @param size                      Number of bytes to read.
     * @throws Exception                On error.
     */
    void readOrFail(void *data, size_t size);

    /**
     * @param size                      Number of bytes to skip.
     * @return                          Number of bytes actually skipped. A return value that's less than `size` signals
     *                                  end of stream.
     * @throws Exception                On error.
     */
    [[nodiscard]] virtual size_t skip(size_t size) = 0;

    /**
     * Closes this input stream. Reading from a closed stream will result in undefined behavior.
     *
     * Does nothing if the stream is already closed.
     *
     * @throws Exception                On error.
     */
    virtual void close() = 0;
};
