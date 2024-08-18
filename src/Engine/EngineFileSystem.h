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
 *
 * It's possible to replace both user and data filesystems, `dataFs` and `userFs` are proxies that you can call 
 * `setBase` on. To reset the proxies to their default state, call `dataFs()->setBase(defaultDataFs())`, and the same
 * for the user fs.
 */
class EngineFileSystem {
 public:
    EngineFileSystem(std::string_view dataPath, std::string_view userPath);
    virtual ~EngineFileSystem();

    /**
     * @returns                         Default base of this filesystem, as constructed. Call 
     *                                  `fs->setBase(fs->defaultBase())` to restore this file system to the same state
     *                                  it was in after construction.
     */
    ProxyFileSystem *dataFs();

    ProxyFileSystem *userFs();

    FileSystem *defaultDataFs();

    FileSystem *defaultUserFs();

 private:
    std::unique_ptr<EmbeddedFileSystem> _dataEmbeddedFs;
    std::unique_ptr<DirectoryFileSystem> _dataDirFs;
    std::unique_ptr<LowercaseFileSystem> _dataDirLowercaseFs;
    std::unique_ptr<MergingFileSystem> _defaultDataFs;
    std::unique_ptr<DirectoryFileSystem> _defaultUserFs;
    std::unique_ptr<ProxyFileSystem> _dataFs;
    std::unique_ptr<ProxyFileSystem> _userFs;
};

extern ProxyFileSystem *dfs;
extern ProxyFileSystem *ufs;

