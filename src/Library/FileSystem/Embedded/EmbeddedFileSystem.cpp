#include "EmbeddedFileSystem.h"

#include <vector>
#include <string>
#include <memory>

#include "Utility/Streams/MemoryInputStream.h"
#include "Utility/String/Encoding.h"
#include "Utility/String/Join.h"

EmbeddedFileSystem::EmbeddedFileSystem(cmrc::embedded_filesystem base, std::string_view displayName) : _base(base), _displayName(displayName) {}

EmbeddedFileSystem::~EmbeddedFileSystem() = default;

bool EmbeddedFileSystem::_exists(NormalPathView path) const {
    return _base.exists(std::string(path.str()));
}

FileStat EmbeddedFileSystem::_stat(NormalPathView path) const {
    std::string stringPath(path.str());

    if (!_base.exists(stringPath))
        return {};

    if (_base.is_directory(stringPath))
        return FileStat(FILE_DIRECTORY, 0);

    cmrc::file file = _base.open(stringPath);
    return FileStat(FILE_REGULAR, file.size());
}

void EmbeddedFileSystem::_ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const {
    for (const cmrc::directory_entry &entry : _base.iterate_directory(std::string(path.str())))
        entries->push_back(DirectoryEntry(entry.filename(), entry.is_file() ? FILE_REGULAR : FILE_DIRECTORY));
}

Blob EmbeddedFileSystem::_read(NormalPathView path) const {
    cmrc::file file = _base.open(std::string(path.str()));
    return Blob::view(file.begin(), file.size()).withDisplayPath(displayPath(path));
}

std::unique_ptr<InputStream> EmbeddedFileSystem::_openForReading(NormalPathView path) const {
    cmrc::file file = _base.open(std::string(path.str()));
    return std::make_unique<MemoryInputStream>(file.begin(), file.size(), displayPath(path));
}

std::string EmbeddedFileSystem::_displayPath(NormalPathView path) const {
    return join(_displayName, "://", txt::encodedToUtf8(path.str(), ENCODING_UTF8)); // Replaces invalid UTF8.
}
