#include "InputStream.h"

#include "Utility/Exception.h"

void InputStream::readOrFail(void *data, size_t size) {
    size_t bytes = read(data, size);
    if (bytes != size)
        throw Exception("Failed to read the requested number of bytes from a stream, requested {}, got {}", size, bytes);
}
