#include "FileInputStream.h"

#include <cassert>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>

#include "Utility/Exception.h"
#include "Utility/System/Os.h"

#ifdef _WINDOWS
#   define ftello _ftelli64
#   define fseeko _fseeki64
#endif

FileInputStream::FileInputStream(const NativePath &path, size_t bufferSize) {
    open(path, bufferSize);
}

FileInputStream::~FileInputStream() {
    destroy();
}

void FileInputStream::open(const NativePath &path, size_t bufferSize) {
    assert(bufferSize > 0);

    std::string displayString = os::absolute(path).displayString(); // Absolute, so that it's still meaningful in logs.

    // Wide fopen on Windows - the narrow one converts the path per the C locale.
#ifdef _WINDOWS
    _file = _wfopen(path.toStdPath().c_str(), L"rb");
#else
    _file = fopen(path.toStdPath().c_str(), "rb");
#endif
    if (!_file)
        Exception::throwFromErrno(displayString);

    // Disable libc buffering, we manage our own buffer.
    if (setvbuf(_file, nullptr, _IONBF, 0) != 0)
        Exception::throwFromErrno(displayString);

    // Compute file size at open time.
    if (fseeko(_file, 0, SEEK_END) != 0)
        Exception::throwFromErrno(displayString);
    int64_t fileEnd = ftello(_file);
    if (fileEnd == -1)
        Exception::throwFromErrno(displayString);
    if (fseeko(_file, 0, SEEK_SET) != 0)
        Exception::throwFromErrno(displayString);

    _bufSize = bufferSize;
    base_type::open({}, fileEnd, displayString);
}

size_t FileInputStream::_underflow(void *data, size_t size, Buffer *buffer) {
    assert(buffer->remaining() == 0);

    if (!_buf)
        _buf = std::make_unique<char[]>(_bufSize);

    if (size < _bufSize) {
        // Small read/skip/refill: fill the internal buffer.
        size_t bytesRead = fread(_buf.get(), 1, _bufSize, _file);
        if (bytesRead == 0 && !feof(_file))
            Exception::throwFromErrno(displayPath());
        buffer->reset(_buf.get(), _buf.get(), _buf.get() + bytesRead);
        if (data) {
            return buffer->read(data, std::min(size, bytesRead));
        } else {
            return buffer->skip(std::min(size, bytesRead));
        }
    } else if (data) {
        // Large read: direct fread.
        size_t bytesRead = fread(data, 1, size, _file);
        if (bytesRead == 0 && !feof(_file))
            Exception::throwFromErrno(displayPath());
        return bytesRead;
    } else {
        // Large skip: seek.
        size_t bytesToSkip = std::min(size, this->size() - position());
        if (bytesToSkip > 0 && fseeko(_file, bytesToSkip, SEEK_CUR) != 0)
            Exception::throwFromErrno(displayPath());
        return bytesToSkip;
    }
}

void FileInputStream::_close(bool canThrow) {
    assert(isOpen());

    int status = fclose(_file);
    _file = nullptr;
    _buf.reset();
    _bufSize = 0;
    if (status != 0 && canThrow)
        Exception::throwFromErrno(displayPath());
    // TODO(captainurist): !canThrow => log OR attach

    base_type::_close(canThrow);
}
