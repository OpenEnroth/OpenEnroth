#include "EmbeddedFileSystem.h"

#include <vector>
#include <string>
#include <memory>

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/String/Encoding.h"
#include "Utility/String/Join.h"

EmbeddedFileSystem::EmbeddedFileSystem(cmrc::embedded_filesystem base, std::string_view displayName) : _base(base), _displayName(displayName) {}

EmbeddedFileSystem::~EmbeddedFileSystem() = default;

static std::string cmrcPath(FileSystemPathView path) {
    return std::string(path.string().substr(1)); // cmrc paths have no leading separator, and the root is empty.
}

bool EmbeddedFileSystem::_exists(FileSystemPathView path) const {
    return _base.exists(cmrcPath(path));
}

FileStat EmbeddedFileSystem::_stat(FileSystemPathView path) const {
    std::string stringPath = cmrcPath(path);

    if (!_base.exists(stringPath))
        return {};

    if (_base.is_directory(stringPath))
        return FileStat(FILE_DIRECTORY, 0);

    cmrc::file file = _base.open(stringPath);
    return FileStat(FILE_REGULAR, file.size());
}

void EmbeddedFileSystem::_ls(FileSystemPathView path, std::vector<DirectoryEntry> *entries) const {
    for (const cmrc::directory_entry &entry : _base.iterate_directory(cmrcPath(path)))
        entries->push_back(DirectoryEntry(entry.filename(), entry.is_file() ? FILE_REGULAR : FILE_DIRECTORY));
}

Blob EmbeddedFileSystem::_read(FileSystemPathView path) const {
    cmrc::file file = _base.open(cmrcPath(path));
    return Blob::view(file.begin(), file.size()).withDisplayPath(displayPath(path));
}

std::unique_ptr<InputStream> EmbeddedFileSystem::_openForReading(FileSystemPathView path) const {
    cmrc::file file = _base.open(cmrcPath(path));
    return std::make_unique<MemoryInputStream>(file.begin(), file.size(), displayPath(path));
}

std::string EmbeddedFileSystem::_displayPath(FileSystemPathView path) const {
    return join(_displayName, ":/", txt::encodedToUtf8(path.string(), ENCODING_UTF8)); // Replaces invalid UTF8.
}
