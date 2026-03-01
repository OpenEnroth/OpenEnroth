#pragma once

#include <string>
#include <string_view>

#include "OutputStream.h"

/**
 * Output stream that writes into a `std::string`.
 */
class StringOutputStream : public OutputStream {
 public:
    StringOutputStream() = default;

    /**
     * @param target                    String to write into. Must outlive this stream.
     * @param displayPath               Display path for error reporting.
     */
    explicit StringOutputStream(std::string *target, std::string_view displayPath = {});
    ~StringOutputStream();

    /**
     * Opens the stream for writing into the given string.
     *
     * @param target                    String to write into. Must outlive this stream.
     * @param displayPath               Display path for error reporting.
     */
    void open(std::string *target, std::string_view displayPath = {});

 private:
    virtual void _overflow(const void *data, size_t size, void **bufferStart, void **bufferEnd) override;
    virtual void _flush() override;
    virtual void _close() override;

    void closeInternal();

 private:
    std::string *_target = nullptr;
};
