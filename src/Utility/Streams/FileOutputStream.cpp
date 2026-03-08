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

    std::string absPath = absolute(std::filesystem::path(path)).generic_string();
    _file = fopen(absPath.c_str(), "wb");
    if (!_file)
        Exception::throwFromErrno(absPath);

    // Disable libc buffering, we manage our own buffer.
    if (setvbuf(_file, nullptr, _IONBF, 0) != 0)
        Exception::throwFromErrno(absPath);

    _bufSize = bufferSize;
    base_type::open({}, absPath);
}

void FileOutputStream::_overflow(const void *data, size_t size, Buffer *buffer) {
    if (size < _bufSize) {
        // Small write: fill current buffer, write it all out, put the tail into a fresh buffer.
        size_t head = buffer->write(data, buffer->remaining());
        writeBuffer(*buffer);
        data = static_cast<const char *>(data) + head;
        size -= head;
        if (!_buf)
            _buf = std::make_unique<char[]>(_bufSize);
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
        buffer->write(data, size);
    } else {
        // Large write: write out current buffer, then write data directly.
        writeBuffer(*buffer);
        if (fwrite(data, size, 1, _file) != 1)
            Exception::throwFromErrno(displayPath());
        if (_buf)
            buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
    }
}

void FileOutputStream::_flush(Buffer *buffer) {
    writeBuffer(*buffer);
    buffer->commit();
    if (fflush(_file) != 0)
        Exception::throwFromErrno(displayPath());
}

void FileOutputStream::_close(Buffer *buffer) {
    assert(isOpen());
    writeBuffer(*buffer);
    closeInternal(true);
    base_type::_close(buffer);
}

void FileOutputStream::writeBuffer(const Buffer &buffer) {
    if (size_t bytesBuffered = buffer.used())
        if (fwrite(buffer.start(), bytesBuffered, 1, _file) != 1)
            Exception::throwFromErrno(displayPath());
}

void FileOutputStream::closeInternal(bool canThrow) {
    if (!isOpen())
        return;

    int status = fclose(_file);
    if (status != 0 && canThrow) // TODO(captainurist): !canThrow => log OR attach
        Exception::throwFromErrno(displayPath());
    _file = nullptr;
    _buf.reset();
    _bufSize = 0;
}
