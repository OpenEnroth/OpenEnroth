#include "MergingFileSystem.h"

#include <cassert>
#include <utility>
#include <unordered_map>
#include <ranges>
#include <string>
#include <vector>
#include <memory>

#include "Library/FileSystem/Interface/FileSystemException.h"

MergingFileSystem::MergingFileSystem(std::span<FileSystem *> bases) {
    assert(!bases.empty());

    std::ranges::copy(bases, std::back_inserter(_bases));
}

MergingFileSystem::~MergingFileSystem() = default;

bool MergingFileSystem::_exists(const FileSystemPath &path) const {
    for (FileSystem *base : _bases)
        if (base->exists(path))
            return true;
    return false;
}

FileStat MergingFileSystem::_stat(const FileSystemPath &path) const {
    for (FileSystem *base : _bases)
        if (FileStat result = base->stat(path))
            return result;
    return {};
}

std::vector<DirectoryEntry> MergingFileSystem::_ls(const FileSystemPath &path) const {
    std::unordered_map<std::string, FileType> mapping;

    bool hasOne = false;
    for (FileSystem *base : _bases) {
        if (!base->exists(path))
            continue;

        hasOne = true;
        for (DirectoryEntry &entry : base->ls(path))
            mapping.emplace(std::move(entry.name), entry.type); // emplace doesn't replace if the element exists.
    }

    if (!hasOne)
        throw FileSystemException(FileSystemException::LS_FAILED_PATH_DOESNT_EXIST, path);

    std::vector<DirectoryEntry> result;
    for (auto &&[name, type] : mapping)
        result.emplace_back(name, type);
    return result;
}

Blob MergingFileSystem::_read(const FileSystemPath &path) const {
    return locateForReading(path, true)->read(path);
}

void MergingFileSystem::_write(const FileSystemPath &path, const Blob &data) {
    return _bases[0]->write(path, data);
}

std::unique_ptr<InputStream> MergingFileSystem::_openForReading(const FileSystemPath &path) const {
    return locateForReading(path, true)->openForReading(path);
}

std::unique_ptr<OutputStream> MergingFileSystem::_openForWriting(const FileSystemPath &path) {
    return _bases[0]->openForWriting(path);
}

void MergingFileSystem::_rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) {
    FileSystem *base = locateForReading(srcPath, false);
    if (base == _bases[0]) {
        base->rename(srcPath, dstPath);
        _remove(srcPath); // Need to clean up other filesystems after a rename.
    } else {
        FileSystem::_rename(srcPath, dstPath); // Base filesystem will try to copy & remove.
    }
}

bool MergingFileSystem::_remove(const FileSystemPath &path) {
    bool result = false;
    for (FileSystem *base : _bases)
        result |= base->remove(path);
    return result;
}

FileSystem *MergingFileSystem::locateForReading(const FileSystemPath &path, bool shouldExist) const {
    for (FileSystem *base : _bases)
        if (base->exists(path))
            return base;

    if (shouldExist)
        throw FileSystemException(FileSystemException::READ_FAILED_PATH_DOESNT_EXIST, path);

    return nullptr;
}
