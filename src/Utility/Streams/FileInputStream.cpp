#include "FileInputStream.h"

#include <cassert>
#include <cstdio>
#include <algorithm> // For std::min.
#include <string>
#include <filesystem>

#include "Utility/Exception.h"
#include "Utility/UnicodeCrt.h"

#ifdef _WINDOWS
#   define ftello _ftelli64
#   define fseeko _fseeki64
#endif

FileInputStream::FileInputStream(std::string_view path) {
    open(path);
}

FileInputStream::~FileInputStream() {
    closeInternal(false);
}

void FileInputStream::open(std::string_view path) {
    assert(UnicodeCrt::isInitialized()); // Otherwise fopen on Windows will choke on UTF-8 paths.

    _path = absolute(std::filesystem::path(path)).generic_string();
    _file = fopen(_path.c_str(), "rb");
    if (!_file)
        Exception::throwFromErrno(_path);
}

size_t FileInputStream::read(void *data, size_t size) {
    assert(isOpen()); // Reading from a closed stream is UB.

    size_t result = fread(data, 1, size, _file);
    if (result == size)
        return result;

    if (feof(_file))
        return result;

    Exception::throwFromErrno(_path);
}

size_t FileInputStream::skip(size_t size) {
    assert(isOpen());

    if (size < 1024) {
        char buf[1024];
        return read(buf, size);
    }

    int64_t pos = ftello(_file);
    if (pos == -1)
        Exception::throwFromErrno(_path);

    if (fseeko(_file, 0, SEEK_END) != 0)
        Exception::throwFromErrno(_path);

    int64_t end = ftello(_file);
    if (end == -1)
        Exception::throwFromErrno(_path);

    int64_t newPos = std::min(pos + static_cast<int64_t>(size), end);
    if (fseeko(_file, newPos, SEEK_SET) != 0)
        Exception::throwFromErrno(_path);

    return newPos - pos;
}

void FileInputStream::close() {
    closeInternal(true);
}

std::string FileInputStream::displayPath() const {
    return _path;
}

void FileInputStream::seek(ssize_t pos) {
    assert(isOpen());

    if (fseeko(_file, 0, SEEK_END) != 0)
        Exception::throwFromErrno(_path);

    int64_t end = ftello(_file);
    if (end == -1)
        Exception::throwFromErrno(_path);

    pos = std::clamp<ssize_t>(pos, 0, end); // Seek beyond EOF just seeks to EOF.

    if (fseeko(_file, pos, SEEK_SET) != 0)
        Exception::throwFromErrno(_path);
}

void FileInputStream::closeInternal(bool canThrow) {
    if (!isOpen())
        return;

    int status = fclose(_file);
    _file = nullptr;
    if (status != 0 && canThrow)
        Exception::throwFromErrno(_path);
    // TODO(captainurist): !canThrow => log OR attach
    _path = {};
}
