#include "NullFileSystem.h"

#include <memory>
#include <vector>
#include <string>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/String/Join.h"

bool NullFileSystem::_exists(FileSystemPathView path) const {
    return false;
}

FileStat NullFileSystem::_stat(FileSystemPathView path) const {
    return {};
}

void NullFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    if (path.isEmpty()) {
        entries->clear();
        return;
    }
    FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
}

Blob NullFileSystem::_read(FileSystemPathView path) const {
    reportReadError(path);
}

std::unique_ptr<InputStream> NullFileSystem::_openForReading(FileSystemPathView path) const {
    reportReadError(path);
}

std::string NullFileSystem::_displayPath(FileSystemPathView path) const {
    return join("null://", path.string());
}

[[noreturn]] void NullFileSystem::reportReadError(FileSystemPathView path) const {
    FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
}
