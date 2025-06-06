#pragma once

#include <vector>
#include <memory>
#include <string>

#include <cmrc/cmrc.hpp>

#include "Library/FileSystem/Interface/ReadOnlyFileSystem.h"

class EmbeddedFileSystem : public ReadOnlyFileSystem {
 public:
    explicit EmbeddedFileSystem(cmrc::embedded_filesystem base, std::string_view displayName);
    virtual ~EmbeddedFileSystem();

 private:
    virtual bool _exists(FileSystemPathView path) const override;
    virtual FileStat _stat(FileSystemPathView path) const override;
    virtual void _ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const override;
    virtual Blob _read(FileSystemPathView path) const override;
    virtual std::unique_ptr<InputStream> _openForReading(FileSystemPathView path) const override;
    virtual std::string _displayPath(FileSystemPathView path) const override;

 private:
    cmrc::embedded_filesystem _base;
    std::string _displayName;
};
