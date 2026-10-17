#include "NullFileSystem.h"

#include <memory>
#include <vector>
#include <string>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/String/Encoding.h"
#include "Utility/String/Join.h"

bool NullFileSystem::_exists(NormalPathView path) const {
    return false;
}

FileStat NullFileSystem::_stat(NormalPathView path) const {
    return {};
}

void NullFileSystem::_ls(NormalPathView path, std::vector<DirectoryEntry> *entries) const {
    if (path.isEmpty()) {
        entries->clear();
        return;
    }
    FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
}

Blob NullFileSystem::_read(NormalPathView path) const {
    reportReadError(path);
}

std::unique_ptr<InputStream> NullFileSystem::_openForReading(NormalPathView path) const {
    reportReadError(path);
}

std::string NullFileSystem::_displayPath(NormalPathView path) const {
    return join("null://", txt::encodedToUtf8(path.str(), ENCODING_UTF8)); // Replaces invalid UTF8.
}

[[noreturn]] void NullFileSystem::reportReadError(PathView path) const {
    FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
}
