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
    void mask(PathView path);
    bool unmask(std::string_view path);
    bool unmask(PathView path);
    void clearMasks();

 private:
    bool isMasked(PathView path) const;

    virtual bool _exists(PathView path) const override;
    virtual FileStat _stat(PathView path) const override;
    virtual void _ls(PathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(PathView path) const override;
    virtual void _write(PathView path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(PathView path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(PathView path) override;
    virtual bool _remove(PathView path) override;
    virtual std::string _displayPath(PathView path) const override;

 private:
    FileSystemTrie<bool> _masks;
};
