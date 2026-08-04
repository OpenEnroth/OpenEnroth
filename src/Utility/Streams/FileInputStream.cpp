#include "FileInputStream.h"

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <filesystem>

#include "Utility/Exception.h"
#include "Utility/ScopeGuard.h"
#include "Utility/UnicodeCrt.h"

#include "FileApi.h"

FileInputStream::FileInputStream(std::string_view path, size_t bufferSize) {
    open(path, bufferSize);
}

FileInputStream::~FileInputStream() {
    destroy();
}

void FileInputStream::open(std::string_view path, size_t bufferSize) {
    assert(UnicodeCrt::isInitialized()); // Otherwise fopen on Windows will choke on UTF-8 paths.
    assert(bufferSize > 0);

    if (isOpen())
        close();
    assert(!_buf); // Sized for the old `_bufSize`, so it has to be gone before that changes.

    std::string absolutePath = absolute(std::filesystem::path(path)).generic_string();
    FILE *file = detail::fileApi->openFile(absolutePath.c_str(), "rb");
    if (!file)
        Exception::throwFromErrno(absolutePath);
    MM_AT_SCOPE_EXIT(if (file) detail::fileApi->closeFile(file)); // Don't leak it if anything below throws.

    // Disable libc buffering, we manage our own buffer.
    if (detail::fileApi->setBuffering(file, nullptr, _IONBF, 0) != 0)
        Exception::throwFromErrno(absolutePath);

    // Compute file size at open time.
    if (detail::fileApi->seek(file, 0, SEEK_END) != 0)
        Exception::throwFromErrno(absolutePath);
    int64_t fileEnd = detail::fileApi->tell(file);
    if (fileEnd == -1)
        Exception::throwFromErrno(absolutePath);
    if (detail::fileApi->seek(file, 0, SEEK_SET) != 0)
        Exception::throwFromErrno(absolutePath);

    _file = std::exchange(file, nullptr);
    _bufSize = bufferSize;
    base_type::open({}, fileEnd, absolutePath);
}

size_t FileInputStream::_underflow(void *data, size_t size, Buffer *buffer) {
    assert(buffer->remaining() == 0);

    if (!_buf)
        _buf = std::make_unique<char[]>(_bufSize);

    if (size < _bufSize) {
        // Small read/skip/refill: fill the internal buffer.
        detail::fileApi->clearError(_file); // So that `ferror` below means "this read failed", not "some earlier read failed".
        size_t bytesRead = detail::fileApi->readBytes(_buf.get(), 1, _bufSize, _file);
        if (bytesRead == 0 && detail::fileApi->checkError(_file)) // Partial reads are returned as-is, the error resurfaces on the next call.
            Exception::throwFromErrno(displayPath());
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + bytesRead);
        if (data) {
            return buffer->read(data, std::min(size, bytesRead));
        } else {
            return buffer->skip(std::min(size, bytesRead));
        }
    } else if (data) {
        // Large read: direct fread.
        detail::fileApi->clearError(_file); // So that `ferror` below means "this read failed", not "some earlier read failed".
        size_t bytesRead = detail::fileApi->readBytes(data, 1, size, _file);
        if (bytesRead == 0 && detail::fileApi->checkError(_file)) // Partial reads are returned as-is, the error resurfaces on the next call.
            Exception::throwFromErrno(displayPath());
        return bytesRead;
    } else {
        // Large skip. Seek over what the file provably holds, then read and discard the rest - a seek-derived
        // length understates the readable data for some files, and `skip` has to agree with `read`.
        int64_t cur = detail::fileApi->tell(_file);
        if (cur == -1)
            Exception::throwFromErrno(displayPath());
        if (detail::fileApi->seek(_file, 0, SEEK_END) != 0)
            Exception::throwFromErrno(displayPath());
        int64_t end = detail::fileApi->tell(_file);
        size_t bytesSkipped = 0;
        if (end != -1) {
            uint64_t bytesLeft = end > cur ? end - cur : 0;
            bytesSkipped = static_cast<size_t>(std::min<uint64_t>(size, bytesLeft));
        }
        if (end == -1 || detail::fileApi->seek(_file, cur + bytesSkipped, SEEK_SET) != 0) {
            // The measuring seek above has moved the offset to the end, and `position()` hasn't - put it back, or a
            // caller that catches the exception reads EOF where its data is.
            int error = errno;
            (void) detail::fileApi->seek(_file, cur, SEEK_SET);
            Exception::throwFromErrno(error, displayPath());
        }

        while (bytesSkipped < size) {
            detail::fileApi->clearError(_file); // So that `ferror` below means "this read failed", not "some earlier read failed".
            size_t bytesRead = detail::fileApi->readBytes(_buf.get(), 1, std::min(size - bytesSkipped, _bufSize), _file);
            bytesSkipped += bytesRead;
            if (bytesRead == 0) {
                // Once the offset has moved, throwing would strand it past what `position()` reports. Partial skips
                // are returned as-is instead, and the error resurfaces on the next call.
                if (bytesSkipped == 0 && detail::fileApi->checkError(_file))
                    Exception::throwFromErrno(displayPath());
                break; // End of stream, or an error that the next call will report.
            }
        }
        return bytesSkipped;
    }
}

void FileInputStream::_close(bool canThrow) {
    assert(isOpen());

    // Tear down first, throw last - this stream has to end up closed either way.
    std::string path = displayPath(); // `base_type::_close` clears it.
    int error = detail::fileApi->closeFile(_file) != 0 ? errno : 0;
    _file = nullptr;
    _buf.reset();
    _bufSize = 0;

    base_type::_close(canThrow);

    if (error != 0 && canThrow) // TODO(captainurist): !canThrow => log OR attach
        Exception::throwFromErrno(error, path);
}
