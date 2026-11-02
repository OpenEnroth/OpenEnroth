#include "EmbeddedFileSystem.h"

#include <vector>
#include <string>
#include <memory>

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/String/Encoding.h"
#include "Utility/String/Join.h"

EmbeddedFileSystem::EmbeddedFileSystem(cmrc::embedded_filesystem base, std::string_view displayName) : _base(base), _displayName(displayName) {}

EmbeddedFileSystem::~EmbeddedFileSystem() = default;

static std::string cmrcPath(NormalPathView path) {
    return std::string(path.str().substr(1)); // cmrc paths have no leading separator, and the root is empty.
}

bool EmbeddedFileSystem::_exists(NormalPathView path) const {
    return _base.exists(cmrcPath(path));
}

FileStat EmbeddedFileSystem::_stat(NormalPathView path) const {
    std::string stringPath = cmrcPath(path);

    if (!_base.exists(stringPath))
        return {};

    if (_base.is_directory(stringPath))
        return FileStat(FILE_DIRECTORY, 0);

    cmrc::file file = _base.open(stringPath);
    return FileStat(FILE_REGULAR, file.size());
}

void EmbeddedFileSystem::_ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const {
    for (const cmrc::directory_entry &entry : _base.iterate_directory(cmrcPath(path)))
        entries->push_back(DirectoryEntry(entry.filename(), entry.is_file() ? FILE_REGULAR : FILE_DIRECTORY));
}

Blob EmbeddedFileSystem::_read(NormalPathView path) const {
    cmrc::file file = _base.open(cmrcPath(path));
    return Blob::view(file.begin(), file.size()).withDisplayPath(_displayPath(path));
}

std::unique_ptr<InputStream> EmbeddedFileSystem::_openForReading(NormalPathView path) const {
    cmrc::file file = _base.open(cmrcPath(path));
    return std::make_unique<MemoryInputStream>(file.begin(), file.size(), _displayPath(path));
}

std::string EmbeddedFileSystem::_displayPath(NormalPathView path) const {
    return join(_displayName, ":/", txt::encodedToUtf8(path.str(), ENCODING_UTF8)); // Replaces invalid UTF8.
}
