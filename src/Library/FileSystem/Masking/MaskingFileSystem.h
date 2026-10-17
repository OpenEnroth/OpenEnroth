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

    virtual bool _exists(NormalPathView path) const override;
    virtual FileStat _stat(NormalPathView path) const override;
    virtual void _ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(NormalPathView path) const override;
    virtual void _write(NormalPathView path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(NormalPathView path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(NormalPathView path) override;
    virtual bool _remove(NormalPathView path) override;
    virtual std::string _displayPath(NormalPathView path) const override;

 private:
    FileSystemTrie<bool> _masks;
};
