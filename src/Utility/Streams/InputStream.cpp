#include "InputStream.h"

#include <stdexcept>

#include "Utility/Format.h"

void InputStream::ReadOrFail(void *data, size_t size) {
    size_t read = Read(data, size);
    if (read != size)
        throw std::runtime_error(fmt::format("Failed to read the requested number of bytes from a stream, requested {}, got {}", size, read));
}
