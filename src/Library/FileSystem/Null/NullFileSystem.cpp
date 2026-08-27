#include "NullFileSystem.h"

#include <memory>
#include <vector>
#include <string>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/String/Encoding.h"
#include "Utility/String/Join.h"

bool NullFileSystem::_exists(PathView path) const {
    return false;
}

FileStat NullFileSystem::_stat(PathView path) const {
    return {};
}

void NullFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    if (path.isEmpty()) {
        entries->clear();
        return;
    }
    FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
}

Blob NullFileSystem::_read(PathView path) const {
    reportReadError(path);
}

std::unique_ptr<InputStream> NullFileSystem::_openForReading(PathView path) const {
    reportReadError(path);
}

std::string NullFileSystem::_displayPath(PathView path) const {
    return join("null://", txt::encodedToUtf8(path.string(), ENCODING_UTF8)); // Replaces invalid UTF8.
}

[[noreturn]] void NullFileSystem::reportReadError(PathView path) const {
    FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
}
