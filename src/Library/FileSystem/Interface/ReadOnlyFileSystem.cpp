#include "ReadOnlyFileSystem.h"

#include <memory> // NOLINT: Linter going insane here for some reason.

#include "FileSystemException.h"

void ReadOnlyFileSystem::_write(NormalPathView path, const Blob &data) {
    reportWriteError(path);
}

std::unique_ptr<OutputStream> ReadOnlyFileSystem::_openForWriting(NormalPathView path) {
    reportWriteError(path);
}

bool ReadOnlyFileSystem::_remove(NormalPathView path) {
    if (!_exists(path))
        return false;

    FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
}

void ReadOnlyFileSystem::reportWriteError(PathView path) const {
    FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
}
