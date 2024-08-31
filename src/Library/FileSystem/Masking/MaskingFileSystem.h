#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Proxy/ProxyFileSystem.h"
#include "Library/FileSystem/Trie/FileSystemTrie.h"

/**
 * Proxy filesystem that supports masking out certain parts of the underlying filesystem.
 */
class MaskingFileSystem : public ProxyFileSystem {
 public:
    explicit MaskingFileSystem(FileSystem *base = nullptr);
    virtual ~MaskingFileSystem();

    void mask(std::string_view path);
    void mask(const FileSystemPath &path);
    bool unmask(std::string_view path);
    bool unmask(const FileSystemPath &path);
    void clearMasks();

 private:
    bool isMasked(const FileSystemPath &path) const;

    virtual bool _exists(const FileSystemPath &path) const override;
    virtual FileStat _stat(const FileSystemPath &path) const override;
    virtual void _ls(const FileSystemPath &path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(const FileSystemPath &path) const override;
    virtual void _write(const FileSystemPath &path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(const FileSystemPath &path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(const FileSystemPath &path) override;
    virtual void _rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) override;
    virtual bool _remove(const FileSystemPath &path) override;
    virtual std::string _displayPath(const FileSystemPath &path) const override;

 private:
    FileSystemTrie<bool> _masks;
};
