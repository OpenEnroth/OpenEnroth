#include "FileOutputStream.h"

#include <cassert>

#include "Utility/Exception.h"

FileOutputStream::FileOutputStream(std::string_view path) : path_(path) {
    file_ = fopen(path_.c_str(), "wb");
    if (!file_)
        Exception::throwFromErrno(path_);
}

FileOutputStream::~FileOutputStream() {
    CloseInternal(false);
}

void FileOutputStream::Write(const void *data, size_t size) {
    assert(file_); // Writing into a closed stream is UB.

    if (fwrite(data, size, 1, file_) != 1)
        Exception::throwFromErrno(path_);
}

void FileOutputStream::Flush() {
    assert(file_); // Flushing a closed stream is UB.

    if (fflush(file_) != 0)
        Exception::throwFromErrno(path_);
}

void FileOutputStream::Close() {
    CloseInternal(true);
}

void FileOutputStream::CloseInternal(bool canThrow) {
    if (!file_)
        return;

    int status = fclose(file_);
    file_ = nullptr;
    if (status != 0 && canThrow)
        Exception::throwFromErrno(path_);
    // TODO(captainurist): !canThrow => log OR attach
}
