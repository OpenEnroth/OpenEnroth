#include "MergingFileSystem.h"

#include <cassert>
#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <ranges>
#include <tuple>

#include "Library/FileSystem/Interface/FileSystemException.h"
#include "Library/FileSystem/Null/NullFileSystem.h"

MergingFileSystem::MergingFileSystem(std::vector<const FileSystem *> bases) {
    _bases = std::move(bases);
}

MergingFileSystem::~MergingFileSystem() = default;

bool MergingFileSystem::_exists(PathView path) const {
    assert(path.isNormalized());
    for (const FileSystem *base : _bases)
        if (existsOf(base, path))
            return true;
    return false;
}

FileStat MergingFileSystem::_stat(PathView path) const {
    assert(path.isNormalized());
    bool dirFound = false;
    for (const FileSystem *base : _bases) {
        FileStat stat = statOf(base, path);
        if (stat.type == FILE_REGULAR)
            return stat; // Return the first file found, if any.
        if (stat.type == FILE_DIRECTORY)
            dirFound = true;
    }
    return dirFound ? FileStat(FILE_DIRECTORY, 0) : FileStat();
}

void MergingFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    assert(path.isNormalized());
    std::vector<DirectoryEntry> buffer;

    bool hasOne = false;
    for (const FileSystem *base : _bases) {
        if (statOf(base, path).type != FILE_DIRECTORY)
            continue;

        // We will throw here if the folder was deleted between stat() and ls() calls. That's probably OK.
        hasOne = true;

        buffer.clear(); // The forwarder appends, where the public two-arg ls used to clear for us.
        lsOf(base, path, &buffer);
        std::ranges::move(buffer, std::back_inserter(*entries));
    }

    if (!hasOne && !path.isEmpty())
        FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);

    // Note that we don't need std::stable_sort here b/c no fs-specific data is exposed by the entries.
    std::ranges::sort(*entries);
    auto [tailStart, tailEnd] = std::ranges::unique(*entries);
    entries->erase(tailStart, tailEnd);
}

Blob MergingFileSystem::_read(PathView path) const {
    assert(path.isNormalized());
    return locateForReading(path)->read(path);
}

std::unique_ptr<InputStream> MergingFileSystem::_openForReading(PathView path) const {
    assert(path.isNormalized());
    return locateForReading(path)->openForReading(path);
}

std::string MergingFileSystem::_displayPath(PathView path) const {
    if (_bases.empty())
        return NullFileSystem().displayPath(path); // Empty merging FS is basically a NullFileSystem.

    // TODO(captainurist): This is not ideal, we might want to know ALL merged paths, e.g. see
    //                     ScriptingSystem::_initPackageTable. But the API that we have here doesn't allow that.
    for (const FileSystem *base : _bases)
        if (statOf(base, path).type != FILE_INVALID)
            return displayPathOf(base, path);

    return displayPathOf(_bases[0], path);
}

const FileSystem *MergingFileSystem::locateForReading(PathView path) const {
    const FileSystem *result = locateForReadingOrNull(path);
    if (result == nullptr)
        FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
    return result;
}

const FileSystem *MergingFileSystem::locateForReadingOrNull(PathView path) const {
    for (const FileSystem *base : _bases)
        if (statOf(base, path).type == FILE_REGULAR)
            return base;

    return nullptr;
}
