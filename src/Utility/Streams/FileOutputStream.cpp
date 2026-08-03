#include "FileOutputStream.h"

#include <cassert>
#include <cerrno>
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
        close(); // Drops `_buf`, which is sized for the current `_bufSize`. Can throw if the flush fails.

    std::string absPath = absolute(std::filesystem::path(path)).generic_string();
    FILE *file = fopen(absPath.c_str(), "wb");
    if (!file)
        Exception::throwFromErrno(absPath);
    MM_AT_SCOPE_EXIT(if (file) fclose(file)); // Don't leak it if anything below throws.

    // Disable libc buffering, we manage our own buffer.
    if (setvbuf(file, nullptr, _IONBF, 0) != 0)
        Exception::throwFromErrno(absPath);

    assert(!_buf); // Dropped by `close()` above.
    _file = std::exchange(file, nullptr); // Disarms the guard above.
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
        if (fwrite(data, 1, size, _file) != size) // Byte-wise, see `writeBuffer`.
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
    int closeError = 0;

    {
        // Runs on every path, including when the write below throws - otherwise the destructor re-enters and both
        // rewrites the buffer and closes a second time.
        MM_AT_SCOPE_EXIT(
            closeError = fclose(_file) != 0 ? errno : 0;
            _file = nullptr;
            _buf.reset();
            _bufSize = 0;
            base_type::_close(buffer, canThrow));

        writeBuffer(buffer, canThrow);
    }

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
