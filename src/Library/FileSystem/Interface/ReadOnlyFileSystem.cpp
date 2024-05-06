#include "ReadOnlyFileSystem.h"

#include <memory> // NOLINT: Linter going insane here for some reason.

#include "FileSystemException.h"

void ReadOnlyFileSystem::_write(const FileSystemPath &path, const Blob &data) {
    reportWriteError(path);
}

std::unique_ptr<OutputStream> ReadOnlyFileSystem::_openForWriting(const FileSystemPath &path) {
    reportWriteError(path);
}

void ReadOnlyFileSystem::_rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) {
    throw FileSystemException(FileSystemException::RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);
}

bool ReadOnlyFileSystem::_remove(const FileSystemPath &path) {
    if (!_exists(path))
        return false;

    throw FileSystemException(FileSystemException::REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
}

void ReadOnlyFileSystem::reportWriteError(const FileSystemPath &path) const {
    throw FileSystemException(FileSystemException::WRITE_FAILED_PATH_NOT_WRITEABLE, path);
}
