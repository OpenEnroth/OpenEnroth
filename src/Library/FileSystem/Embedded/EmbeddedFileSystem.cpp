#include "EmbeddedFileSystem.h"

#include <cassert>
#include <vector>
#include <string>
#include <memory>

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/String/Encoding.h"
#include "Utility/String/Join.h"

EmbeddedFileSystem::EmbeddedFileSystem(cmrc::embedded_filesystem base, std::string_view displayName) : _base(base), _displayName(displayName) {}

EmbeddedFileSystem::~EmbeddedFileSystem() = default;

bool EmbeddedFileSystem::_exists(PathView path) const {
    assert(path.isNormalized());
    return _base.exists(std::string(path.string()));
}

FileStat EmbeddedFileSystem::_stat(PathView path) const {
    assert(path.isNormalized());
    std::string stringPath(path.string());

    if (!_base.exists(stringPath))
        return {};

    if (_base.is_directory(stringPath))
        return FileStat(FILE_DIRECTORY, 0);

    cmrc::file file = _base.open(stringPath);
    return FileStat(FILE_REGULAR, file.size());
}

void EmbeddedFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    assert(path.isNormalized());
    for (const cmrc::directory_entry &entry : _base.iterate_directory(std::string(path.string())))
        entries->push_back(DirectoryEntry(entry.filename(), entry.is_file() ? FILE_REGULAR : FILE_DIRECTORY));
}

Blob EmbeddedFileSystem::_read(PathView path) const {
    assert(path.isNormalized());
    cmrc::file file = _base.open(std::string(path.string()));
    return Blob::view(file.begin(), file.size()).withDisplayPath(displayPath(path));
}

std::unique_ptr<InputStream> EmbeddedFileSystem::_openForReading(PathView path) const {
    assert(path.isNormalized());
    cmrc::file file = _base.open(std::string(path.string()));
    return std::make_unique<MemoryInputStream>(file.begin(), file.size(), displayPath(path));
}

std::string EmbeddedFileSystem::_displayPath(PathView path) const {
    return join(_displayName, "://", path.displayString());
}
