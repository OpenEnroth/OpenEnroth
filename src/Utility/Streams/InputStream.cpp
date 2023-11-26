#include "InputStream.h"

#include <cassert>
#include <algorithm>

#include "Utility/Exception.h"

void InputStream::readOrFail(void *data, size_t size) {
    size_t bytes = read(data, size);
    if (bytes != size)
        throw Exception("Failed to read the requested number of bytes from a stream, requested {}, got {}", size, bytes);
}

std::string InputStream::readAll(size_t maxSize) {
    size_t chunkSize = 1024;
    size_t maxChunkSize = 16 * 1024 * 1024;

    std::string result;
    while (true) {
        size_t oldSize = result.size();
        size_t newSize = std::min(oldSize + chunkSize, maxSize);
        result.resize(newSize); // TODO(captainurist): Use C++23 resize_and_overwrite.

        size_t bytesRequested = newSize - oldSize;
        size_t bytesRead = read(result.data() + oldSize, bytesRequested);
        if (bytesRead < bytesRequested) {
            result.resize(oldSize + bytesRead);
            return result;
        }

        if (result.size() == maxSize)
            return result;
        assert(result.size() < maxSize);

        if (chunkSize < maxChunkSize)
            chunkSize *= 2;
    }
}

void InputStream::skipOrFail(size_t size) {
    size_t bytes = skip(size);
    if (bytes != size)
        throw Exception("Failed to skip the requested number of bytes in a stream, requested {}, got {}", size, bytes);
}
