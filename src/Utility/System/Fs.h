#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "Utility/System/FileStat.h"
#include "Utility/System/Path.h"

namespace fs {

/**
 * @param path                          Path to check. Never throws, returns `false` on errors.
 * @return                              Whether `path` exists.
 */
[[nodiscard]] bool exists(const Path &path);

/**
 * @param path                          Path to stat. Never throws.
 * @return                              Stats for `path`, or an empty `FileStat` on errors, or if `path` is neither
 *                                      a file nor a directory.
 */
[[nodiscard]] FileStat stat(const Path &path);

/**
 * Lists a directory. Never throws - lists nothing if `path` doesn't exist or isn't a directory, and skips entries
 * that can't be stat'ed, so the result is always in sync with what `stat` returns.
 *
 * @param path                          Path to a directory to list.
 * @return                              Directory entries, in unspecified order. Names are WTF-8 on Windows, byte
 *                                      strings on POSIX.
 */
[[nodiscard]] std::vector<DirectoryEntry> ls(const Path &path);

/**
 * Same as `ls` above, but appends into a vector the caller already has, saving an allocation.
 *
 * @param path                          Path to a directory to list.
 * @param[out] entries                  Vector to append the entries to.
 */
void ls(const Path &path, std::vector<DirectoryEntry> *entries);

/**
 * Removes the file or directory at `path`. A directory is removed with everything that's in it.
 *
 * @param path                          Path to remove.
 * @return                              Whether anything was removed.
 * @throws std::runtime_error           On errors, e.g. missing permissions.
 */
bool remove(const Path &path);

/**
 * Creates the directory at `path`, along with all missing parents. Does nothing if it already exists.
 *
 * @param path                          Path to the directory to create.
 * @throws std::runtime_error           On errors.
 */
void mkdirs(const Path &path);

/**
 * @return                              Current working directory.
 */
[[nodiscard]] Path cwd();

/**
 * @param path                          Path to resolve.
 * @return                              Absolute copy of `path`, resolved against the current directory. An empty path
 *                                      resolves to the current directory itself.
 * @throws Exception                    If the path couldn't be resolved.
 */
[[nodiscard]] Path absolute(const Path &path);

} // namespace fs
