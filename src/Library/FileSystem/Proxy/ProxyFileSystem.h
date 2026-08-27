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
    virtual bool _exists(PathView path) const override;
    virtual FileStat _stat(PathView path) const override;
    virtual void _ls(PathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(PathView path) const override;
    virtual void _write(PathView path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(PathView path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(PathView path) override;
    virtual bool _remove(PathView path) override;
    virtual std::string _displayPath(PathView path) const override;

    FileSystem *nonNullBase() const;

 private:
    FileSystem *_base = nullptr;
};
