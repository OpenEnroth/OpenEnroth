#include "ReadOnlyFileSystem.h"

#include <memory> // NOLINT: Linter going insane here for some reason.

#include "FileSystemException.h"

void ReadOnlyFileSystem::_write(FileSystemPathView path, const Blob &data) {
    reportWriteError(path);
}

std::unique_ptr<OutputStream> ReadOnlyFileSystem::_openForWriting(FileSystemPathView path) {
    reportWriteError(path);
}

void ReadOnlyFileSystem::_rename(FileSystemPathView srcPath, FileSystemPathView dstPath) {
    FileSystemException::raise(this, FS_RENAME_FAILED_DST_NOT_WRITEABLE, srcPath, dstPath);
}

bool ReadOnlyFileSystem::_remove(FileSystemPathView path) {
    if (!_exists(path))
        return false;

    FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
}

void ReadOnlyFileSystem::reportWriteError(FileSystemPathView path) const {
    FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
}
