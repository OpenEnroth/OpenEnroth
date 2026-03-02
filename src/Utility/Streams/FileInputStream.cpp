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
    base_type::open({}, _path);
}

size_t FileInputStream::_underflow(void *data, size_t size, Buffer *buffer) {
    assert(buffer->remaining() == 0);

    if (!_buf)
        _buf = std::make_unique<char[]>(_bufSize);

    if (size < _bufSize) {
        // Small read/skip/refill: fill the internal buffer.
        size_t bytesRead = fread(_buf.get(), 1, _bufSize, _file);
        if (bytesRead == 0 && !feof(_file))
            Exception::throwFromErrno(_path);
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + bytesRead);
        if (data) {
            return buffer->read(data, std::min(size, bytesRead));
        } else {
            return buffer->skip(std::min(size, bytesRead));
        }
    }

    if (data) {
        // Large read: direct fread.
        size_t result = fread(data, 1, size, _file);
        if (result == 0 && !feof(_file))
            Exception::throwFromErrno(_path);
        return result;
    } else {
        // Large skip: seek.
        size_t result = std::min(size, fileRemaining());
        if (result > 0 && fseeko(_file, result, SEEK_CUR) != 0)
            Exception::throwFromErrno(_path);
        return result;
    }
}

size_t FileInputStream::_readAll(std::string *dst, size_t maxSize) {
    assert(isOpen());

    size_t result = std::min(fileRemaining(), maxSize);
    if (result == 0)
        return 0;

    size_t oldSize = dst->size();
    dst->resize_and_overwrite(oldSize + result, [this, oldSize, result](char *buf, size_t n) {
        if (fread(buf + oldSize, result, 1, _file) != 1)
            Exception::throwFromErrno(_path);
        return n;
    });

    return result;
}

void FileInputStream::_close() {
    assert(isOpen());
    closeInternal(true);
    base_type::_close();
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

size_t FileInputStream::fileRemaining() {
    int64_t cur = ftello(_file);
    if (cur == -1)
        Exception::throwFromErrno(_path);

    if (fseeko(_file, 0, SEEK_END) != 0)
        Exception::throwFromErrno(_path);

    int64_t fileEnd = ftello(_file);
    if (fileEnd == -1)
        Exception::throwFromErrno(_path);

    if (fseeko(_file, cur, SEEK_SET) != 0)
        Exception::throwFromErrno(_path);

    return fileEnd - cur;
}
