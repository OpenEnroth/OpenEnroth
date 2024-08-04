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

std::vector<DirectoryEntry> NullFileSystem::_ls(const FileSystemPath &path) const {
    if (path.isEmpty())
        return {};
    throw FileSystemException(FileSystemException::LS_FAILED_PATH_DOESNT_EXIST, path);
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
    throw FileSystemException(FileSystemException::READ_FAILED_PATH_DOESNT_EXIST, path);
}
