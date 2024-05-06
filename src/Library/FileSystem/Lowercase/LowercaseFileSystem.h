#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

#include "Library/FileSystem/Interface/ReadOnlyFileSystem.h"

/**
 * Provides a lowercase view over another `FileSystem`:
 * - Works only with lowercase names, passing names containing uppercase characters to any of the methods will throw.
 * - Provides a lowercase view over the existing files, effectively giving the user a case-insensitive view over
 *   a potentially case-sensitive filesystem.
 * - Is a view, and thus read-only.
 * - Caches the contents of the underlying filesystem in constructor. Call `refresh` to update the cache.
 * - Constructor and `refresh` throw on coflicts.
 */
class LowercaseFileSystem : public ReadOnlyFileSystem {
 public:
    explicit LowercaseFileSystem(FileSystem *base);
    virtual ~LowercaseFileSystem();

    void refresh();

 private:
    virtual bool _exists(const FileSystemPath &path) const override;
    virtual FileStat _stat(const FileSystemPath &path) const override;

    virtual std::vector<DirectoryEntry> _ls(const FileSystemPath &path) const override;

    virtual Blob _read(const FileSystemPath &path) const override;
    virtual std::unique_ptr<InputStream> _openForReading(const FileSystemPath &path) const override;

    void refresh(const FileSystemPath &path, const FileSystemPath &basePath);

    struct FileSystemEntry {
        FileSystemPath path; // Path in the base filesystem.
        FileStat stat;
        std::vector<DirectoryEntry> ls;
    };

    const FileSystemEntry &fileEntryForReading(const FileSystemPath &path) const;
    const FileSystemEntry &dirEntryForListing(const FileSystemPath &path) const;

 private:
    FileSystem *_base = nullptr;
    std::unordered_map<FileSystemPath, FileSystemEntry> _entryByPath;
};
