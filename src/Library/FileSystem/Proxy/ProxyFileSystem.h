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
    virtual bool _exists(NormalPathView path) const override;
    virtual FileStat _stat(NormalPathView path) const override;
    virtual void _ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(NormalPathView path) const override;
    virtual void _write(NormalPathView path, const Blob &data) override;
    virtual std::unique_ptr<InputStream> _openForReading(NormalPathView path) const override;
    virtual std::unique_ptr<OutputStream> _openForWriting(NormalPathView path) override;
    virtual bool _remove(NormalPathView path) override;
    virtual std::string _displayPath(NormalPathView path) const override;

    FileSystem *nonNullBase() const;

 private:
    FileSystem *_base = nullptr;
};
