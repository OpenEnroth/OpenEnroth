#include "Blob.h"

#include <cstdio>
#include <string>
#include <memory>
#include <utility>
#include <filesystem>

#include <mio/mmap.hpp>

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Exception.h"

#include "FreeDeleter.h"

Blob Blob::subBlob(size_t offset, size_t size) const {
    if (offset >= _size || size == 0)
        return Blob();

    Blob result;
    result._data = static_cast<const char *>(_data) + offset;
    result._size = std::min(size, _size - offset);
    result._state = _state;
    return result;
}

Blob Blob::fromMalloc(const void *data, size_t size) {
    if (!data)
        return Blob();

    Blob result;
    result._data = data;
    result._size = size;
    result._state = std::shared_ptr<void>(const_cast<void *>(data), FreeDeleter());
    return result;
}

Blob Blob::fromMalloc(std::unique_ptr<void, FreeDeleter> data, size_t size) {
    return fromMalloc(data.release(), size);
}

Blob Blob::fromFile(std::string_view path) {
    std::filesystem::path absolutePath = absolute(std::filesystem::path(path));
    std::string pathString = absolutePath.generic_string();

    // On Mac mapping an empty file throws, so we need to provide a workaround.
    std::error_code error;
    uintmax_t size = std::filesystem::file_size(absolutePath, error);
    if (!error && size == 0)
        return Blob().withDisplayPath(pathString);

    // On Windows mio::mmap_source expects UTF8-encoded paths. If the file doesn't exist, std::system_error is thrown.
    std::shared_ptr<mio::mmap_source> mmap = std::make_shared<mio::mmap_source>(pathString);

    Blob result;
    result._data = mmap->data();
    result._size = mmap->size();
    result._state = std::move(mmap);
    result._displayPath = std::move(pathString);
    return result;
}

Blob Blob::fromString(std::string string) {
    if (string.empty())
        return Blob();

    std::shared_ptr<std::string> state = std::make_shared<std::string>(std::move(string));

    Blob result;
    result._data = state->data();
    result._size = state->size();
    result._state = std::move(state);
    return result;
}

Blob Blob::copy(const void *data, size_t size) { // NOLINT: this is not std::copy
    if (size == 0)
        return Blob();

    std::unique_ptr<void, FreeDeleter> memory(malloc(size)); // We don't handle allocation failures.
    memcpy(memory.get(), data, size);
    return fromMalloc(std::move(memory), size);
}

Blob Blob::copy(const Blob &other) { // NOLINT: this is not std::copy.
    return copy(other.data(), other.size()).withDisplayPath(other.displayPath()); // NOLINT: seriously, this is not std::copy.
}

Blob Blob::view(const void *data, size_t size) {
    Blob result;
    result._data = data;
    result._size = size;
    // state is empty!
    return result;
}

Blob Blob::view(std::string_view data) {
    return view(data.data(), data.size());
}

Blob Blob::read(FILE *file, size_t size) {
    if (size == 0)
        return Blob();

    std::unique_ptr<void, FreeDeleter> memory(malloc(size));

    size_t read = fread(memory.get(), size, 1, file);
    if (read != 1)
        throw Exception("Failed to read {} bytes from file", size);

    return fromMalloc(std::move(memory), size);
}

Blob Blob::read(InputStream *stream, size_t size) {
    if (size == 0)
        return Blob();

    std::unique_ptr<void, FreeDeleter> memory(malloc(size));
    stream->readOrFail(memory.get(), size);
    return fromMalloc(std::move(memory), size).withDisplayPath(stream->displayPath());
}

Blob Blob::concat(const Blob &l, const Blob &r) {
    size_t lsize = l.size();
    size_t rsize = r.size();
    if (lsize == 0 && rsize == 0) {
        return Blob();
    } else if (lsize == 0) {
        return Blob::share(r);
    } else if (rsize == 0) {
        return Blob::share(l);
    }

    std::unique_ptr<void, FreeDeleter> memory(malloc(lsize + rsize));

    memcpy(memory.get(), l.data(), lsize);
    memcpy(static_cast<char *>(memory.get()) + lsize, r.data(), rsize);

    return fromMalloc(std::move(memory), lsize + rsize);
}

Blob Blob::share(const Blob &other) {
    Blob result;
    result._data = other._data;
    result._size = other._size;
    result._state = other._state;
    result._displayPath = other._displayPath;
    return result;
}
