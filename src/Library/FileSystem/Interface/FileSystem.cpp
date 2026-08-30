#include "FileSystem.h"

#include <vector>
#include <memory>
#include <string>

#include "Utility/System/PathView.h"

#include "FileSystemException.h"

// A path that this file system can act on: relative, so that it names something under the root rather than a
// location of its own, and non-escaping, so that it doesn't climb out. Normal form is what makes the check
// meaningful - "a/../.." is escaping and "a/../b" isn't, and only normalizing tells them apart cheaply.
static bool isAccessible(PathView path) {
    return !path.isAbsolute() && !path.isEscaping();
}

// Every public entry point starts here. The private _ methods take the result and assume it, so this is the one
// place the invariant is established. Scans first and only allocates when there is something to rewrite, which in
// practice there never is - callers pass paths that are already normal.
class NormalizedPath {
 public:
    explicit NormalizedPath(PathView path) : _view(path) {
        if (!path.isNormalized()) {
            _owned = Path(path).normalized();
            _view = _owned;
        }
    }

    NormalizedPath(const NormalizedPath &) = delete; // _view can point into _owned.
    NormalizedPath &operator=(const NormalizedPath &) = delete;

    operator PathView() const { return _view; } // NOLINT: intentionally implicit.

    [[nodiscard]] bool isEmpty() const { return _view.isEmpty(); }
    [[nodiscard]] bool isAbsolute() const { return _view.isAbsolute(); }
    [[nodiscard]] bool isEscaping() const { return _view.isEscaping(); }

 private:
    Path _owned;
    PathView _view;
};

bool FileSystem::exists(std::string_view path) const {
    return exists(Path(path));
}

bool FileSystem::exists(PathView path) const {
    NormalizedPath normalPath(path);
    if (normalPath.isEmpty())
        return true; // Root always exists.
    if (!isAccessible(normalPath))
        return false; // Inaccessible paths behave as if they don't exist.
    return _exists(normalPath);
}

FileStat FileSystem::stat(std::string_view path) const {
    return stat(Path(path));
}

FileStat FileSystem::stat(PathView path) const {
    NormalizedPath normalPath(path);
    if (normalPath.isEmpty())
        return FileStat(FILE_DIRECTORY, 0);
    if (!isAccessible(normalPath))
        return FileStat();
    return _stat(normalPath);
}

std::vector<DirectoryEntry> FileSystem::ls(std::string_view path) const {
    return ls(Path(path));
}

std::vector<DirectoryEntry> FileSystem::ls(PathView path) const {
    NormalizedPath normalPath(path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    std::vector<DirectoryEntry> result;
    _ls(normalPath, &result);
    return result;
}

void FileSystem::ls(std::string_view path, std::vector<DirectoryEntry> *entries) const {
    ls(Path(path), entries);
}

void FileSystem::ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    NormalizedPath normalPath(path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_LS_FAILED_PATH_NOT_ACCESSIBLE, path);
    entries->clear();
    _ls(normalPath, entries);
}

Blob FileSystem::read(std::string_view path) const {
    return read(Path(path));
}

Blob FileSystem::read(PathView path) const {
    NormalizedPath normalPath(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _read(normalPath);
}

void FileSystem::write(std::string_view path, const Blob &data) {
    return write(Path(path), data);
}

void FileSystem::write(PathView path, const Blob &data) {
    NormalizedPath normalPath(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    _write(normalPath, data);
}

std::unique_ptr<InputStream> FileSystem::openForReading(std::string_view path) const {
    return openForReading(Path(path));
}

std::unique_ptr<InputStream> FileSystem::openForReading(PathView path) const {
    NormalizedPath normalPath(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_READ_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_READ_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForReading(normalPath);
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(std::string_view path) {
    return openForWriting(Path(path));
}

std::unique_ptr<OutputStream> FileSystem::openForWriting(PathView path) {
    NormalizedPath normalPath(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_IS_DIR, path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _openForWriting(normalPath);
}

bool FileSystem::remove(std::string_view path) {
    return remove(Path(path));
}

bool FileSystem::remove(PathView path) {
    NormalizedPath normalPath(path);
    if (normalPath.isEmpty())
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_WRITEABLE, path);
    if (!isAccessible(normalPath))
        FileSystemException::raise(this, FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE, path);
    return _remove(normalPath);
}

std::string FileSystem::displayPath(std::string_view path) const {
    return displayPath(Path(path));
}

std::string FileSystem::displayPath(PathView path) const {
    // Normalizes like everything else, because the _ methods assume normal form. It deliberately does not validate
    // though - raising a FileSystemException formats the offending path through here, so refusing one would recurse.
    return _displayPath(NormalizedPath(path));
}
