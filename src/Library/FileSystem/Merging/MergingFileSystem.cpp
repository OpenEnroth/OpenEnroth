#include "MergingFileSystem.h"

#include <cassert>
#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <ranges>
#include <tuple>

#include "Library/FileSystem/Interface/FileSystemException.h"

MergingFileSystem::MergingFileSystem(std::vector<const FileSystem *> bases) {
    _bases = std::move(bases);
}

MergingFileSystem::~MergingFileSystem() = default;

bool MergingFileSystem::_exists(const FileSystemPath &path) const {
    for (const FileSystem *base : _bases)
        if (base->exists(path))
            return true;
    return false;
}

FileStat MergingFileSystem::_stat(const FileSystemPath &path) const {
    bool dirFound = false;
    for (const FileSystem *base : _bases) {
        FileStat stat = base->stat(path);
        if (stat.type == FILE_REGULAR)
            return stat; // Return the first file found, if any.
        if (stat.type == FILE_DIRECTORY)
            dirFound = true;
    }
    return dirFound ? FileStat(FILE_DIRECTORY, 0) : FileStat();
}

std::vector<DirectoryEntry> MergingFileSystem::_ls(const FileSystemPath &path) const {
    std::vector<DirectoryEntry> result;

    bool hasOne = false;
    for (const FileSystem *base : _bases) {
        if (base->stat(path).type != FILE_DIRECTORY)
            continue;

        // We will throw here if the folder was deleted between stat() and ls() calls. That's probably OK.
        hasOne = true;
        std::ranges::move(base->ls(path), std::back_inserter(result));
    }

    if (!hasOne) {
        if (path.isEmpty())
            return {};
        throw FileSystemException(FileSystemException::LS_FAILED_PATH_DOESNT_EXIST, path);
    }

    std::ranges::sort(result, [] (const DirectoryEntry &l, const DirectoryEntry &r) {
        return std::tie(l.name, l.type) < std::tie(r.name, r.type);
    });
    auto [tailStart, tailEnd] = std::ranges::unique(result);
    result.erase(tailStart, tailEnd);

    return result;
}

Blob MergingFileSystem::_read(const FileSystemPath &path) const {
    return locateForReading(path)->read(path);
}

std::unique_ptr<InputStream> MergingFileSystem::_openForReading(const FileSystemPath &path) const {
    return locateForReading(path)->openForReading(path);
}

const FileSystem *MergingFileSystem::locateForReading(const FileSystemPath &path) const {
    for (const FileSystem *base : _bases)
        if (base->stat(path).type == FILE_REGULAR)
            return base;

    throw FileSystemException(FileSystemException::READ_FAILED_PATH_DOESNT_EXIST, path);
}
