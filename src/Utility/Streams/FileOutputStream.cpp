#include "FileOutputStream.h"

#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

#include "Utility/Exception.h"
#include "Utility/System/Os.h"

FileOutputStream::FileOutputStream(const NativePath &path, size_t bufferSize) {
    open(path, bufferSize);
}

FileOutputStream::~FileOutputStream() {
    destroy();
}

void FileOutputStream::open(const NativePath &path, size_t bufferSize) {
    assert(bufferSize > 0);

    std::string displayString = os::absolute(path).displayString(); // Absolute, so that it's still meaningful in logs.

    // Wide fopen on Windows - the narrow one converts the path per the C locale.
#ifdef _WINDOWS
    _file = _wfopen(path.toStdPath().c_str(), L"wb");
#else
    _file = fopen(path.toStdPath().c_str(), "wb");
#endif
    if (!_file)
        Exception::throwFromErrno(displayString);

    // Disable libc buffering, we manage our own buffer.
    if (setvbuf(_file, nullptr, _IONBF, 0) != 0)
        Exception::throwFromErrno(displayString);

    _bufSize = bufferSize;
    base_type::open({}, displayString);
}

void FileOutputStream::_overflow(Buffer *buffer, const void *data, size_t size) {
    if (size < _bufSize) {
        // Small write: fill current buffer, write it all out, put the tail into a fresh buffer.
        size_t head = buffer->write(data, buffer->remaining());
        writeBuffer(*buffer, true);
        data = static_cast<const char *>(data) + head;
        size -= head;
        if (!_buf)
            _buf = std::make_unique<char[]>(_bufSize);
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
        buffer->write(data, size);
    } else {
        // Large write: write out current buffer, then write data directly.
        writeBuffer(*buffer, true);
        if (fwrite(data, size, 1, _file) != 1)
            Exception::throwFromErrno(displayPath());
        if (_buf)
            buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
    }
}

void FileOutputStream::_flush(Buffer *buffer) {
    writeBuffer(*buffer, true);
    buffer->commit();
    if (fflush(_file) != 0)
        Exception::throwFromErrno(displayPath());
}

void FileOutputStream::_close(Buffer *buffer, bool canThrow) {
    assert(isOpen());

    writeBuffer(*buffer, canThrow);

    int status = fclose(_file);
    if (status != 0 && canThrow) // TODO(captainurist): !canThrow => log OR attach
        Exception::throwFromErrno(displayPath());
    _file = nullptr;
    _buf.reset();
    _bufSize = 0;

    base_type::_close(buffer, canThrow);
}

void FileOutputStream::writeBuffer(const Buffer &buffer, bool canThrow) {
    if (size_t bytesBuffered = buffer.used())
        if (fwrite(buffer.start(), bytesBuffered, 1, _file) != 1 && canThrow)
            Exception::throwFromErrno(displayPath());
}
