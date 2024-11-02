#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Library/FileSystem/Interface/FileSystem.h"

class ProxyFileSystem : public FileSystem {
 public:
    explicit ProxyFileSystem(FileSystem *base = nullptr): _base(base) {}

    FileSystem *base() const {
        return _base;
    }

    void setBase(FileSystem *base) {
        _base = base;
    }

 protected:
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

    FileSystem *nonNullBase() const;

 private:
    FileSystem *_base = nullptr;
};
