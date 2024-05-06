#include "LowercaseFileSystem.h"

#include <cassert>
#include <memory>
#include <vector>
#include <string>
#include <utility>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/String/Ascii.h"
#include "Utility/MapAccess.h"
#include "Utility/Exception.h"

LowercaseFileSystem::LowercaseFileSystem(FileSystem *base): _base(base) {
    assert(_base);
    refresh();
}

LowercaseFileSystem::~LowercaseFileSystem() = default;

void LowercaseFileSystem::refresh() {
    try {
        _entryByPath.clear();
        refresh(FileSystemPath(), FileSystemPath());
    } catch (...) {
        // refresh() failed - just clear everything & rethrow. This can happen because of conflicts, or if the
        // underlying filesystem is acting up.
        _entryByPath.clear();
        throw;
    }
}

bool LowercaseFileSystem::_exists(const FileSystemPath &path) const {
    return _entryByPath.contains(path);
}

FileStat LowercaseFileSystem::_stat(const FileSystemPath &path) const {
    const FileSystemEntry *entry = valuePtr(_entryByPath, path);
    return entry ? entry->stat : FileStat();
}

std::vector<DirectoryEntry> LowercaseFileSystem::_ls(const FileSystemPath &path) const {
    return dirEntryForListing(path).ls;
}

Blob LowercaseFileSystem::_read(const FileSystemPath &path) const {
    return _base->read(fileEntryForReading(path).path);
}

std::unique_ptr<InputStream> LowercaseFileSystem::_openForReading(const FileSystemPath &path) const {
    return _base->openForReading(fileEntryForReading(path).path);
}

void LowercaseFileSystem::refresh(const FileSystemPath &path, const FileSystemPath &basePath) {
    std::vector<DirectoryEntry> entries = _base->ls(basePath);

    for (DirectoryEntry &entry : entries) {
        std::string lowerName = ascii::toLower(entry.name);
        FileSystemPath entryPath = path.appended(lowerName);
        FileSystemPath baseEntryPath = basePath.appended(entry.name);

        if (_entryByPath.contains(entryPath)) // It's a conflict!
            throw Exception("Can't refresh a lowercase filesystem because paths '{}' and '{}' are conflicting.", baseEntryPath, _entryByPath[entryPath].path);

        FileSystemEntry &cacheEntry = _entryByPath[entryPath];
        cacheEntry.path = baseEntryPath;
        cacheEntry.stat = _base->stat(baseEntryPath);

        if (entry.type == FILE_DIRECTORY)
            refresh(entryPath, baseEntryPath); // This will set cacheEntry.ls.

        entry.name = std::move(lowerName);
    }

    _entryByPath[path].ls = std::move(entries);
}

const LowercaseFileSystem::FileSystemEntry &LowercaseFileSystem::fileEntryForReading(const FileSystemPath &path) const {
    const FileSystemEntry *result = valuePtr(_entryByPath, path);
    if (!result)
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_DOESNT_EXIST, path);
    if (result->stat.type == FILE_DIRECTORY)
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_IS_DIR, path);
    return *result;
}

const LowercaseFileSystem::FileSystemEntry &LowercaseFileSystem::dirEntryForListing(const FileSystemPath &path) const {
    const FileSystemEntry *result = valuePtr(_entryByPath, path);
    if (!result)
        throw FileSystemException(FileSystemException::LS_FAILED_PATH_DOESNT_EXIST, path);
    if (result->stat.type == FILE_REGULAR)
        throw FileSystemException(FileSystemException::LS_FAILED_PATH_IS_FILE, path);
    return *result;
}
