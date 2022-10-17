#include "FileOutputStream.h"

#include <cassert>
#include <system_error>

FileOutputStream::FileOutputStream(const std::string &path) : path_(path) {
    file_ = fopen(path_.c_str(), "rb");
    if (!file_)
        ThrowFromErrno();
}

FileOutputStream::~FileOutputStream() {
    CloseInternal(false);
}

void FileOutputStream::Write(const void *data, size_t size) {
    assert(file_); // Writing into a closed stream is UB.

    if (fwrite(data, size, 1, file_) != 1)
        ThrowFromErrno();
}

void FileOutputStream::Flush() {
    assert(file_); // Flushing a closed stream is UB.

    if (fflush(file_) != 0)
        ThrowFromErrno();
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
        ThrowFromErrno();
}

void FileOutputStream::ThrowFromErrno() {
    assert(errno != 0);

    throw std::system_error(errno, std::generic_category(), path_);
}
