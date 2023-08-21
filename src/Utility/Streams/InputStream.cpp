#include "InputStream.h"

#include <cassert>
#include <algorithm>

#include "Utility/Exception.h"
#include "fmt/core.h"

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
        result.resize(oldSize + chunkSize); // TODO(captainurist): Use C++23 resize_and_overwrite.

        chunkSize = std::min(maxSize - result.size(), chunkSize);
        size_t bytesRead = read(result.data() + oldSize, chunkSize);
        if (bytesRead < chunkSize) {
            result.resize(result.size() + bytesRead);
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
