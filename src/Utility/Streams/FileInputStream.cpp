#include "FileInputStream.h"

#include <cassert>
#include <algorithm> // For std::min.

#include "Utility/Exception.h"

#ifdef _WINDOWS
#   define ftello _ftelli64
#   define fseeko _fseeki64
#endif

FileInputStream::FileInputStream(std::string_view path): _path(path) {
    _file = fopen(_path.c_str(), "rb");
    if (!_file)
        Exception::throwFromErrno(_path);
}

FileInputStream::~FileInputStream() {
    CloseInternal(false);
}

size_t FileInputStream::Read(void *data, size_t size) {
    assert(_file); // Reading from a closed stream is UB.

    size_t result = fread(data, 1, size, _file);
    if (result == size)
        return result;

    if (feof(_file))
        return result;

    Exception::throwFromErrno(_path);
}

size_t FileInputStream::Skip(size_t size) {
    assert(_file);

    if (size < 1024) {
        char buf[1024];
        return Read(buf, size);
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

void FileInputStream::Close() {
    CloseInternal(true);
}

void FileInputStream::CloseInternal(bool canThrow) {
    if (!_file)
        return;

    int status = fclose(_file);
    _file = nullptr;
    if (status != 0 && canThrow)
        Exception::throwFromErrno(_path);
    // TODO(captainurist): !canThrow => log OR attach
}
