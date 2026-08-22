#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "Utility/System/NativePath.h"

enum class FileType {
    FILE_INVALID, // Returned by `stat` calls if file doesn't exist.
    FILE_REGULAR,
    FILE_DIRECTORY,
};
using enum FileType;

struct FileStat {
    FileStat() = default;
    FileStat(FileType type, std::int64_t size) : type(type), size(size) {}

    FileType type = FILE_INVALID; // Invalid means file doesn't exist.
    std::int64_t size = 0; // Always zero for directories.

    explicit operator bool() const {
        return type != FILE_INVALID;
    }

    friend bool operator==(const FileStat &l, const FileStat &r) = default;
};

struct DirectoryEntry {
    DirectoryEntry() = default;
    DirectoryEntry(std::string name, FileType type) : name(std::move(name)), type(type) {}

    std::string name;
    FileType type = FILE_INVALID; // Never invalid when returned from `ls` calls.

    // Make entries sortable. If two entries have the same name, then files go before directories.
    friend std::strong_ordering operator<=>(const DirectoryEntry &l, const DirectoryEntry &r) = default;
};

namespace os {

// Path-taking functions below also have an `AsciiLiteral` overload - C++ allows a single user-defined conversion,
// so a string literal passed as an argument wouldn't otherwise reach `NativePath`'s literal constructor.

/**
 * @param path                          Path to check. Never throws, returns `false` on errors.
 * @return                              Whether `path` exists.
 */
[[nodiscard]] bool exists(const NativePath &path);
[[nodiscard]] inline bool exists(AsciiLiteral path) { return exists(NativePath(path)); }

/**
 * @param path                          Path to stat. Never throws.
 * @return                              Stats for `path`, or an empty `FileStat` on errors, or if `path` is neither
 *                                      a file nor a directory.
 */
[[nodiscard]] FileStat stat(const NativePath &path);
[[nodiscard]] inline FileStat stat(AsciiLiteral path) { return stat(NativePath(path)); }

/**
 * Lists a directory. Never throws - lists nothing if `path` doesn't exist or isn't a directory, and skips entries
 * that can't be stat'ed, so the result is always in sync with what `stat` returns.
 *
 * @param path                          Path to a directory to list.
 * @param[out] entries                  Vector to append the entries to.
 * @return                              Directory entries, in unspecified order. Names are WTF-8 on Windows, byte
 *                                      strings on POSIX.
 */
[[nodiscard]] std::vector<DirectoryEntry> ls(const NativePath &path);
[[nodiscard]] inline std::vector<DirectoryEntry> ls(AsciiLiteral path) { return ls(NativePath(path)); }
void ls(const NativePath &path, std::vector<DirectoryEntry> *entries);
inline void ls(AsciiLiteral path, std::vector<DirectoryEntry> *entries) { ls(NativePath(path), entries); }

/**
 * Removes the file or directory at `path`. A directory is removed with everything that's in it.
 *
 * @param path                          Path to remove.
 * @return                              Whether anything was removed.
 * @throws std::runtime_error           On errors, e.g. missing permissions.
 */
bool remove(const NativePath &path);
inline bool remove(AsciiLiteral path) { return remove(NativePath(path)); }

/**
 * Creates the directory at `path`, along with all missing parents. Does nothing if it already exists.
 *
 * @param path                          Path to the directory to create.
 * @throws std::runtime_error           On errors.
 */
void mkdirs(const NativePath &path);
inline void mkdirs(AsciiLiteral path) { mkdirs(NativePath(path)); }

/**
 * @return                              Current working directory.
 */
[[nodiscard]] NativePath cwd();

/**
 * @param path                          Path to resolve.
 * @return                              Absolute copy of `path`, resolved against the current directory. An empty path
 *                                      resolves to the current directory itself.
 * @throws Exception                    If the path couldn't be resolved.
 */
[[nodiscard]] NativePath absolute(const NativePath &path);
[[nodiscard]] inline NativePath absolute(AsciiLiteral path) { return absolute(NativePath(path)); }

} // namespace os
