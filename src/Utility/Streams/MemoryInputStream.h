#pragma once

#include <cstddef>
#include <string_view>

#include "InputStream.h"

/**
 * Input stream that reads from a memory region.
 *
 * This is a thin wrapper over `InputStream` that exposes the protected memory-backed constructors.
 */
class MemoryInputStream : public InputStream {
 public:
    MemoryInputStream() = default;

    /**
     * @param data                      Pointer to the data to read from.
     * @param size                      Data size.
     * @param displayPath               Display path for error reporting.
     */
    MemoryInputStream(const void *data, size_t size, std::string_view displayPath = {}) {
        open(data, size, displayPath);
    }

    /**
     * Re-initializes the stream from a new memory region.
     *
     * @param data                      Pointer to the data to read from.
     * @param size                      Data size.
     * @param displayPath               Display path for error reporting.
     */
    void open(const void *data, size_t size, std::string_view displayPath = {}) {
        InputStream::open(data, static_cast<const char *>(data) + size, displayPath);
    }
};
