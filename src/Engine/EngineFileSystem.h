#pragma once

#include <memory>

#include "Library/FileSystem/Proxy/ProxyFileSystem.h"

class MaskingFileSystem;
class MergingFileSystem;
class EmbeddedFileSystem;
class DirectoryFileSystem;
class LowercaseFileSystem;

/**
 * File system manager as used by the engine.
 * 
 * Default data fs configuration:
 * ```
 * - MergingFS
 *     - LowercaseFS
 *         - DirectoryFS for <data-path>.
 *     - EmbeddedFS for scripts and shaders.
 * ```
 * 
 * User fs is just a DirectoryFS.
 *
 * Effectively user fs is the write interface, while data fs is the read interface for everything. Writing through the
 * data fs will fail.
 */
class EngineFileSystem {
 public:
    EngineFileSystem(std::string_view dataPath, std::string_view userPath);
    virtual ~EngineFileSystem();

 private:
    std::unique_ptr<EmbeddedFileSystem> _dataEmbeddedFs;
    std::unique_ptr<DirectoryFileSystem> _dataDirFs;
    std::unique_ptr<LowercaseFileSystem> _dataDirLowercaseFs;
    std::unique_ptr<MergingFileSystem> _defaultDataFs;
    std::unique_ptr<DirectoryFileSystem> _defaultUserFs;
};

extern FileSystem *dfs;
extern FileSystem *ufs;

