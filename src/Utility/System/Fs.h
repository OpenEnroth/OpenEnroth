#pragma once

#include <vector>

#include "Utility/System/FileStat.h"
#include "Utility/System/NativePath.h"

namespace fs {

/**
 * Checks whether `path` exists. Never throws, errors read as a missing path.
 *
 * @param path                          Path to check.
 * @return                              Whether `path` exists. `false` for an empty path.
 */
[[nodiscard]] bool exists(const NativePath &path);

/**
 * Looks up the type and size of `path`. Never throws.
 *
 * @param path                          Path to stat.
 * @return                              Stats for `path`, or an empty `FileStat` on errors, for an empty path, or
 *                                      if `path` is neither a file nor a directory.
 */
[[nodiscard]] FileStat stat(const NativePath &path);

/**
 * Lists a directory. Entries that can't be stat'ed are skipped, so the result is always in sync with what `stat`
 * returns.
 *
 * @param path                          Path to a directory to list.
 * @return                              Directory entries, in unspecified order. Names are WTF-8 on Windows, byte
 *                                      strings on POSIX.
 * @throws Exception                    If the directory can't be opened, e.g. if `path` is empty, doesn't exist,
 *                                      or is a file.
 */
[[nodiscard]] std::vector<DirectoryEntry> ls(const NativePath &path);

/**
 * Same as `ls` above, but appends into a vector the caller already has, saving an allocation.
 *
 * @param path                          Path to a directory to list.
 * @param[out] entries                  Vector to append the entries to.
 * @throws Exception                    Same as the overload above.
 */
void ls(const NativePath &path, std::vector<DirectoryEntry> *entries);

/**
 * Removes the file or directory at `path`. A directory is removed with everything that's in it.
 *
 * @param path                          Path to remove.
 * @return                              Whether anything was removed. `false` for an empty path.
 * @throws Exception                    On errors, e.g. missing permissions.
 */
bool remove(const NativePath &path);

/**
 * Creates the directory at `path`, along with all missing parents. Does nothing if it already exists.
 *
 * @param path                          Path to the directory to create.
 * @throws Exception                    On errors, and for an empty path.
 */
void mkdirs(const NativePath &path);

/**
 * @return                              Current working directory.
 * @throws Exception                    On errors.
 */
[[nodiscard]] NativePath cwd();

/**
 * @return                              Directory for temporary files.
 * @throws Exception                    On errors.
 */
[[nodiscard]] NativePath tmp();

/**
 * Resolves `path` against the current directory.
 *
 * On Windows this isn't lexical. A drive-relative `"C:x"` resolves against the current directory of drive C, which
 * only the OS knows.
 *
 * @param path                          Path to resolve.
 * @return                              Absolute copy of `path`.
 * @throws Exception                    On errors, and for an empty path.
 */
[[nodiscard]] NativePath absolute(const NativePath &path);

} // namespace fs
