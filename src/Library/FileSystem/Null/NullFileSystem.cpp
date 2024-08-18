#include "NullFileSystem.h"

#include <memory>
#include <vector>
#include <string>

#include "Library/FileSystem/Interface/FileSystemException.h"

bool NullFileSystem::_exists(const FileSystemPath &path) const {
    return false;
}

FileStat NullFileSystem::_stat(const FileSystemPath &path) const {
    return {};
}

void NullFileSystem::_ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const {
    if (path.isEmpty()) {
        entries->clear();
        return;
    }
    FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
}

Blob NullFileSystem::_read(const FileSystemPath &path) const {
    reportReadError(path);
}

std::unique_ptr<InputStream> NullFileSystem::_openForReading(const FileSystemPath &path) const {
    reportReadError(path);
}

std::string NullFileSystem::_displayPath(const FileSystemPath &path) const {
    return "null://" + path.string();
}

[[noreturn]] void NullFileSystem::reportReadError(const FileSystemPath &path) const {
    FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
}
