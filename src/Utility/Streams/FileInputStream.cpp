#include "FileInputStream.h"

#include <cassert>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>
#include <filesystem>

#include "Utility/Exception.h"
#include "Utility/UnicodeCrt.h"

#ifdef _WINDOWS
#   define ftello _ftelli64
#   define fseeko _fseeki64
#endif

FileInputStream::FileInputStream(std::string_view path, size_t bufferSize) {
    open(path, bufferSize);
}

FileInputStream::~FileInputStream() {
    closeInternal(false);
}

void FileInputStream::open(std::string_view path, size_t bufferSize) {
    assert(UnicodeCrt::isInitialized()); // Otherwise fopen on Windows will choke on UTF-8 paths.
    assert(bufferSize > 0);

    _path = absolute(std::filesystem::path(path)).generic_string();
    _file = fopen(_path.c_str(), "rb");
    if (!_file)
        Exception::throwFromErrno(_path);

    // Disable libc buffering, we manage our own buffer.
    if (setvbuf(_file, nullptr, _IONBF, 0) != 0)
        Exception::throwFromErrno(_path);

    _bufSize = bufferSize;

    InputStream::open(_path);
}

size_t FileInputStream::_underflow(void *data, size_t size, const void **bufferStart, const void **bufferEnd) {
    assert(bufferRemaining() == 0);

    if (!_buf)
        _buf = std::make_unique<char[]>(_bufSize);

    if (size < _bufSize) {
        // Small read/skip/refill: fill the internal buffer.
        size_t bytesRead = fread(_buf.get(), 1, _bufSize, _file);
        if (bytesRead == 0 && !feof(_file))
            Exception::throwFromErrno(_path);
        size_t toCopy = std::min(size, bytesRead);
        if (data)
            memcpy(data, _buf.get(), toCopy);
        *bufferStart = _buf.get() + toCopy;
        *bufferEnd = _buf.get() + bytesRead;
        return toCopy;
    }

    // Large read/skip: bypass the buffer.
    *bufferStart = nullptr;
    *bufferEnd = nullptr;

    if (data) {
        // Large read: direct fread.
        size_t bytesRead = fread(data, 1, size, _file);
        if (bytesRead == 0 && !feof(_file))
            Exception::throwFromErrno(_path);
        return bytesRead;
    }

    // Large skip: seek.
    int64_t cur = ftello(_file);
    if (cur == -1)
        Exception::throwFromErrno(_path);

    if (fseeko(_file, 0, SEEK_END) != 0)
        Exception::throwFromErrno(_path);

    int64_t fileEnd = ftello(_file);
    if (fileEnd == -1)
        Exception::throwFromErrno(_path);

    int64_t newPos = std::min(cur + static_cast<int64_t>(size), fileEnd);
    if (fseeko(_file, newPos, SEEK_SET) != 0)
        Exception::throwFromErrno(_path);

    return newPos - cur;
}

size_t FileInputStream::_readAll(std::string *dst, size_t maxSize) {
    assert(isOpen());

    int64_t cur = ftello(_file);
    if (cur == -1)
        Exception::throwFromErrno(_path);

    if (fseeko(_file, 0, SEEK_END) != 0)
        Exception::throwFromErrno(_path);

    int64_t fileEnd = ftello(_file);
    if (fileEnd == -1)
        Exception::throwFromErrno(_path);

    size_t remaining = fileEnd - cur;
    size_t toRead = std::min(remaining, maxSize);

    if (fseeko(_file, cur, SEEK_SET) != 0)
        Exception::throwFromErrno(_path);

    if (toRead == 0)
        return 0;

    size_t oldSize = dst->size();
    dst->resize_and_overwrite(oldSize + toRead, [&](char *buf, size_t) {
        if (fread(buf + oldSize, toRead, 1, _file) != 1)
            Exception::throwFromErrno(_path);
        return oldSize + toRead;
    });

    return toRead;
}

void FileInputStream::_close() {
    assert(isOpen());
    closeInternal(true);
    InputStream::_close();
}

void FileInputStream::closeInternal(bool canThrow) {
    if (!isOpen())
        return;

    int status = fclose(_file);
    _file = nullptr;
    _buf.reset();
    _bufSize = 0;
    if (status != 0 && canThrow)
        Exception::throwFromErrno(_path);
    // TODO(captainurist): !canThrow => log OR attach
    _path = {};
}
