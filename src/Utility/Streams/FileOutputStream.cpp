#include "FileOutputStream.h"

#include <cassert>
#include <cstdio>
#include <memory>
#include <string>
#include <filesystem>

#include "Utility/Exception.h"
#include "Utility/UnicodeCrt.h"

FileOutputStream::FileOutputStream(std::string_view path, size_t bufferSize) {
    open(path, bufferSize);
}

FileOutputStream::~FileOutputStream() {
    closeInternal(false);
}

void FileOutputStream::open(std::string_view path, size_t bufferSize) {
    assert(UnicodeCrt::isInitialized()); // Otherwise fopen on Windows will choke on UTF-8 paths.
    assert(bufferSize > 0);

    _path = absolute(std::filesystem::path(path)).generic_string();
    _file = fopen(_path.c_str(), "wb");
    if (!_file)
        Exception::throwFromErrno(_path);

    // Disable libc buffering, we manage our own buffer.
    if (setvbuf(_file, nullptr, _IONBF, 0) != 0)
        Exception::throwFromErrno(_path);

    _bufSize = bufferSize;

    OutputStream::open(_path);
}

void FileOutputStream::_overflow(const void *data, size_t size, void **bufferStart, void **bufferEnd) {
    assert(bufferRemaining() == 0);

    if (_buf) {
        // Flush the full buffer.
        if (fwrite(_buf.get(), _bufSize, 1, _file) != 1)
            Exception::throwFromErrno(_path);
    }

    if (size >= _bufSize) {
        // Large write: write directly.
        if (fwrite(data, size, 1, _file) != 1)
            Exception::throwFromErrno(_path);
        *bufferStart = _buf.get();
        *bufferEnd = _buf.get() + _bufSize;
    } else {
        // Small write: copy into buffer, provide the rest.
        if (!_buf)
            _buf = std::make_unique<char[]>(_bufSize);
        memcpy(_buf.get(), data, size);
        *bufferStart = _buf.get() + size;
        *bufferEnd = _buf.get() + _bufSize;
    }
}

void FileOutputStream::_flush() {
    flushBuffer();

    if (fflush(_file) != 0)
        Exception::throwFromErrno(_path);
}

void FileOutputStream::_close() {
    if (!isOpen())
        return;

    flushBuffer();
    closeInternal(true);
    OutputStream::_close();
}

void FileOutputStream::flushBuffer() {
    assert(isOpen());

    if (!_buf)
        return;

    size_t usedBytes = _bufSize - bufferRemaining();
    if (usedBytes == 0)
        return;

    if (fwrite(_buf.get(), usedBytes, 1, _file) != 1)
        Exception::throwFromErrno(_path);
}

void FileOutputStream::closeInternal(bool canThrow) {
    if (!isOpen())
        return;

    int status = fclose(_file);
    if (status != 0 && canThrow) // TODO(captainurist): !canThrow => log OR attach
        Exception::throwFromErrno(_path);
    _file = nullptr;
    _buf.reset();
    _bufSize = 0;
    _path = {};
}
