#pragma once

#include <vector>

#include "Utility/System/FileStat.h"
#include "Utility/System/NativePath.h"

namespace fs {

/**
 * @param path                          Path to check. Never throws, returns `false` on errors.
 * @return                              Whether `path` exists.
 */
[[nodiscard]] bool exists(const NativePath &path);

/**
 * @param path                          Path to stat. Never throws.
 * @return                              Stats for `path`, or an empty `FileStat` on errors, or if `path` is neither
 *                                      a file nor a directory.
 */
[[nodiscard]] FileStat stat(const NativePath &path);

/**
 * Lists a directory. Never throws. Lists nothing if `path` doesn't exist or isn't a directory, and skips entries
 * that can't be stat'ed, so the result is always in sync with what `stat` returns.
 *
 * @param path                          Path to a directory to list.
 * @return                              Directory entries, in unspecified order. Names are WTF-8 on Windows, byte
 *                                      strings on POSIX.
 */
[[nodiscard]] std::vector<DirectoryEntry> ls(const NativePath &path);

/**
 * Same as `ls` above, but appends into a vector the caller already has, saving an allocation.
 *
 * @param path                          Path to a directory to list.
 * @param[out] entries                  Vector to append the entries to.
 */
void ls(const NativePath &path, std::vector<DirectoryEntry> *entries);

/**
 * Removes the file or directory at `path`. A directory is removed with everything that's in it.
 *
 * @param path                          Path to remove.
 * @return                              Whether anything was removed.
 * @throws std::runtime_error           On errors, e.g. missing permissions.
 */
bool remove(const NativePath &path);

/**
 * Creates the directory at `path`, along with all missing parents. Does nothing if it already exists.
 *
 * @param path                          Path to the directory to create.
 * @throws std::runtime_error           On errors.
 */
void mkdirs(const NativePath &path);

} // namespace fs
