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

 private:
    virtual bool _exists(const FileSystemPath &path) const override;
    virtual FileStat _stat(const FileSystemPath &path) const override;
    virtual std::vector<DirectoryEntry> _ls(const FileSystemPath &path) const override;
    virtual Blob _read(const FileSystemPath &path) const override;
    virtual void _write(const FileSystemPath &path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(const FileSystemPath &path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(const FileSystemPath &path) override;
    virtual void _rename(const FileSystemPath &srcPath, const FileSystemPath &dstPath) override;
    virtual bool _remove(const FileSystemPath &path) override;
    virtual std::string _displayPath(const FileSystemPath &path) const override;

    FileSystem *nonNullBase() const;

 private:
    FileSystem *_base = nullptr;
};
