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

#ifdef _WINDOWS
#   define ftello _ftelli64
#   define fseeko _fseeki64
#endif

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

    std::string absolutePath = absolute(std::filesystem::path(path)).generic_string();
    FILE *file = fopen(absolutePath.c_str(), "rb");
    if (!file)
        Exception::throwFromErrno(absolutePath);
    MM_AT_SCOPE_EXIT(if (file) fclose(file)); // Don't leak it if anything below throws.

    // Disable libc buffering, we manage our own buffer.
    if (setvbuf(file, nullptr, _IONBF, 0) != 0)
        Exception::throwFromErrno(absolutePath);

    // Compute file size at open time.
    if (fseeko(file, 0, SEEK_END) != 0)
        Exception::throwFromErrno(absolutePath);
    int64_t fileEnd = ftello(file);
    if (fileEnd == -1)
        Exception::throwFromErrno(absolutePath);
    if (fseeko(file, 0, SEEK_SET) != 0)
        Exception::throwFromErrno(absolutePath);

    assert(!_buf); // Dropped by `close()` above.
    _file = std::exchange(file, nullptr); // Disarms the guard above.
    _bufSize = bufferSize;
    base_type::open({}, fileEnd, absolutePath);
}

size_t FileInputStream::_underflow(void *data, size_t size, Buffer *buffer) {
    assert(buffer->remaining() == 0);

    if (!_buf)
        _buf = std::make_unique<char[]>(_bufSize);

    if (size < _bufSize) {
        // Small read/skip/refill: fill the internal buffer.
        clearerr(_file); // So that `ferror` below means "this read failed", not "some earlier read failed".
        size_t bytesRead = fread(_buf.get(), 1, _bufSize, _file);
        if (bytesRead < _bufSize && ferror(_file))
            Exception::throwFromErrno(displayPath());
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + bytesRead);
        if (data) {
            return buffer->read(data, std::min(size, bytesRead));
        } else {
            return buffer->skip(std::min(size, bytesRead));
        }
    } else if (data) {
        // Large read: direct fread.
        clearerr(_file); // See above.
        size_t bytesRead = fread(data, 1, size, _file);
        if (bytesRead < size && ferror(_file))
            Exception::throwFromErrno(displayPath());
        return bytesRead;
    } else {
        // Large skip: seek. Saturating, as `position()` can be past `size()` if the file has grown since open.
        size_t bytesToSkip = std::min(size, this->size() > position() ? this->size() - position() : 0);
        if (bytesToSkip > 0 && fseeko(_file, bytesToSkip, SEEK_CUR) != 0)
            Exception::throwFromErrno(displayPath());
        return bytesToSkip;
    }
}

void FileInputStream::_close(bool canThrow) {
    assert(isOpen());

    // Tear down before throwing, or `isOpen()` stays true with a closed `FILE*` and the destructor re-enters.
    std::string path = displayPath(); // `base_type::_close` clears it.
    int error = fclose(_file) != 0 ? errno : 0;
    _file = nullptr;
    _buf.reset();
    _bufSize = 0;

    base_type::_close(canThrow);

    if (error != 0 && canThrow) // TODO(captainurist): !canThrow => log OR attach
        Exception::throwFromErrno(error, path);
}
