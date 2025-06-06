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
    void mask(FileSystemPathView path);
    bool unmask(std::string_view path);
    bool unmask(FileSystemPathView path);
    void clearMasks();

 private:
    bool isMasked(FileSystemPathView path) const;

    virtual bool _exists(FileSystemPathView path) const override;
    virtual FileStat _stat(FileSystemPathView path) const override;
    virtual void _ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(FileSystemPathView path) const override;
    virtual void _write(FileSystemPathView path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(FileSystemPathView path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(FileSystemPathView path) override;
    virtual void _rename(FileSystemPathView srcPath, FileSystemPathView dstPath) override;
    virtual bool _remove(FileSystemPathView path) override;
    virtual std::string _displayPath(FileSystemPathView path) const override;

 private:
    FileSystemTrie<bool> _masks;
};
