#pragma once

#include <cstddef>
#include <string>
#include <string_view>

/**
 * Abstract base class for all data output streams.
 *
 * Compared to `std::ostream` it is:
 * - Non-buffered.
 * - Uses exceptions for error handling.
 * - Has a very simple and intuitive interface consisting of exactly three methods.
 */
class OutputStream {
 public:
    virtual ~OutputStream() {}

    /**
     * Writes provided data into the output stream.
     *
     * @param data                      Pointer to the data to write.
     * @param size                      Data size.
     * @throws Exception                On error.
     */
    virtual void write(const void *data, size_t size) = 0;

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
     * Flushes this output stream if it provides any kind of buffering.
     *
     * @throws Exception                On error.
     */
    virtual void flush() = 0;

    /**
     * Closes this output stream. Writing into or flushing a closed stream will result in undefined behavior.
     *
     * Does nothing if the stream is already closed.
     *
     * @throws Exception                On error.
     */
    virtual void close() = 0;

    /**
     * @return                          Path to the file or resource being written, to be used for debugging and error
     *                                  reporting.
     */
    [[nodiscard]] virtual std::string displayPath() const = 0;
};
