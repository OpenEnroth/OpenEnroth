#include "FileOutputStream.h"

#include <cassert>
#include <cerrno>
#include <exception>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <filesystem>

#include "Utility/Exception.h"
#include "Utility/ScopeGuard.h"
#include "Utility/UnicodeCrt.h"

FileOutputStream::FileOutputStream(std::string_view path, size_t bufferSize) {
    open(path, bufferSize);
}

FileOutputStream::~FileOutputStream() {
    destroy();
}

void FileOutputStream::open(std::string_view path, size_t bufferSize) {
    assert(UnicodeCrt::isInitialized()); // Otherwise fopen on Windows will choke on UTF-8 paths.
    assert(bufferSize > 0);

    if (isOpen())
        close();
    assert(!_buf); // Sized for the old `_bufSize`, so it has to be gone before that changes.

    std::string absPath = absolute(std::filesystem::path(path)).generic_string();
    FILE *file = fopen(absPath.c_str(), "wb");
    if (!file)
        Exception::throwFromErrno(absPath);
    MM_AT_SCOPE_EXIT(if (file) fclose(file)); // Don't leak it if anything below throws.

    // Disable libc buffering, we manage our own buffer.
    if (setvbuf(file, nullptr, _IONBF, 0) != 0)
        Exception::throwFromErrno(absPath);

    _file = std::exchange(file, nullptr);
    _bufSize = bufferSize;
    base_type::open({}, absPath);
}

void FileOutputStream::_overflow(Buffer *buffer, const void *data, size_t size) {
    if (size < _bufSize) {
        // Small write: fill current buffer, write it all out, put the tail into a fresh buffer.
        size_t head = buffer->write(data, buffer->remaining());
        writeBuffer(buffer, true);
        data = static_cast<const char *>(data) + head;
        size -= head;
        if (!_buf)
            _buf = std::make_unique<char[]>(_bufSize);
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
        buffer->write(data, size);
    } else {
        // Large write: write out current buffer, then write data directly.
        writeBuffer(buffer, true);
        if (fwrite(data, 1, size, _file) != size)
            Exception::throwFromErrno(displayPath());
        if (_buf)
            buffer->reset(_buf.get(), _buf.get(), _buf.get() + _bufSize);
    }
}

void FileOutputStream::_flush(Buffer *buffer) {
    writeBuffer(buffer, true); // Drops what went out, so `used()` is 0 on success.
    if (fflush(_file) != 0)
        Exception::throwFromErrno(displayPath());
}

void FileOutputStream::_close(Buffer *buffer, bool canThrow) {
    assert(isOpen());

    std::string path = displayPath(); // `base_type::_close` clears it.

    // Tear down even if the write throws - this stream has to end up closed either way.
    std::exception_ptr writeError;
    try {
        writeBuffer(buffer, canThrow);
    } catch (...) {
        writeError = std::current_exception();
    }

    int closeError = fclose(_file) != 0 ? errno : 0;
    _file = nullptr;
    _buf.reset();
    _bufSize = 0;
    base_type::_close(buffer, canThrow);

    if (writeError)
        std::rethrow_exception(writeError); // The write error is the informative one, so it wins over `closeError`.
    if (closeError != 0 && canThrow) // TODO(captainurist): !canThrow => log OR attach
        Exception::throwFromErrno(closeError, path);
}

void FileOutputStream::writeBuffer(Buffer *buffer, bool canThrow) {
    size_t bytesBuffered = buffer->used();
    if (bytesBuffered == 0)
        return;

    // Byte-wise, because the `fwrite(ptr, size, 1, f)` form returns 0 on a partial write and loses the count. We
    // need it - bytes that did make it out must be dropped, or `_close` writes them again and duplicates them.
    size_t bytesWritten = fwrite(buffer->start(), 1, bytesBuffered, _file);
    buffer->reset(buffer->start() + bytesWritten, buffer->pos(), buffer->end());

    if (bytesWritten != bytesBuffered && canThrow)
        Exception::throwFromErrno(displayPath());
}
