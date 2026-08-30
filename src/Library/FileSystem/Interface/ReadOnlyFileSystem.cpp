#include "ReadOnlyFileSystem.h"

#include <cassert>
#include <memory> // NOLINT: Linter going insane here for some reason.

#include "FileSystemException.h"

void ReadOnlyFileSystem::_write(PathView path, const Blob &data) {
    assert(path.isNormalized());
    reportWriteError(path);
}

std::unique_ptr<OutputStream> ReadOnlyFileSystem::_openForWriting(PathView path) {
    assert(path.isNormalized());
    reportWriteError(path);
}

bool ReadOnlyFileSystem::_remove(PathView path) {
    assert(path.isNormalized());
    if (!_exists(path))
        return false;

    FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
}

void ReadOnlyFileSystem::reportWriteError(PathView path) const {
    FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_WRITEABLE, path);
}
