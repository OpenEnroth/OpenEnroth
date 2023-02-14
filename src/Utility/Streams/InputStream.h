#pragma once

#include <cstddef>

/**
 * Abstract base class for all data input streams.
 *
 * Compared to `std::istream` it is:
 * - Non-buffered.
 * - Uses exceptions for error handling.
 * - Has an extremely simple and intuitive interface consisting of exactly two methods.
 */
class InputStream {
 public:
    virtual ~InputStream() {}

    /**
     * @param data                      Output buffer to write read data into.
     * @param size                      Number of bytes to read.
     * @return                          Number of bytes actually read. A return value that's less than `size` signals
     *                                  end of stream.
     * @throws std::runtime_error       On error.
     */
    virtual size_t Read(void *data, size_t size) = 0;

    /**
     * Reads the requested amount of data from the stream, or fails with an exception if unable to do so.
     *
     * @param data                      Output buffer to write read data into.
     * @param size                      Number of bytes to read.
     */
    void ReadOrFail(void *data, size_t size);

    /**
     * @param size                      Number of bytes to skip.
     * @return                          Number of bytes actually skipped. A return value that's less than `size` signals
     *                                  end of stream.
     * @throws std::runtime_error       On error.
     */
    virtual size_t Skip(size_t size) = 0;
};
