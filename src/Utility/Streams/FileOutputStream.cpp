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
    base_type::open({}, _path);
}

void FileOutputStream::_overflow(const void *data, size_t size, Buffer *buffer) {
    assert(size > buffer->remaining());

    const char *src = static_cast<const char *>(data);

    if (size < _bufSize) {
        // Small write: fill current buffer, write it all out, put the tail into a fresh buffer.
        size_t head = buffer->write(src, buffer->remaining());
        writeBuffer(*buffer);
        if (!_buf)
            _buf = std::make_unique<char[]>(_bufSize);
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
        buffer->write(src + head, size - head);
    } else {
        // Large write: write out current buffer, then write data directly.
        writeBuffer(*buffer);
        if (fwrite(src, size, 1, _file) != 1)
            Exception::throwFromErrno(_path);
        if (_buf)
            buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
    }
}

void FileOutputStream::_flush(Buffer *buffer) {
    writeBuffer(*buffer);
    if (fflush(_file) != 0)
        Exception::throwFromErrno(_path);
    buffer->chop();
}

void FileOutputStream::_close() {
    assert(isOpen());
    writeBuffer(buffer());
    closeInternal(true);
    base_type::_close();
}

void FileOutputStream::writeBuffer(const Buffer &buffer) {
    if (size_t buffered = buffer.used())
        if (fwrite(buffer.start(), buffered, 1, _file) != 1)
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
