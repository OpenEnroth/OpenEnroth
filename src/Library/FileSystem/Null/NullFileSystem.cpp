#include "NullFileSystem.h"

#include <cassert>
#include <memory>
#include <vector>
#include <string>

#include "Library/FileSystem/Interface/FileSystemException.h"

#include "Utility/String/Encoding.h"
#include "Utility/String/Join.h"

bool NullFileSystem::_exists(PathView path) const {
    assert(path.isNormalized());
    return false;
}

FileStat NullFileSystem::_stat(PathView path) const {
    assert(path.isNormalized());
    return {};
}

void NullFileSystem::_ls(PathView path, std::vector<DirectoryEntry> *entries) const {
    assert(path.isNormalized());
    if (path.isEmpty()) {
        entries->clear();
        return;
    }
    FileSystemException::raise(this, FS_LS_FAILED_PATH_DOESNT_EXIST, path);
}

Blob NullFileSystem::_read(PathView path) const {
    assert(path.isNormalized());
    reportReadError(path);
}

std::unique_ptr<InputStream> NullFileSystem::_openForReading(PathView path) const {
    assert(path.isNormalized());
    reportReadError(path);
}

std::string NullFileSystem::_displayPath(PathView path) const {
    return join("null://", path.displayString());
}

[[noreturn]] void NullFileSystem::reportReadError(PathView path) const {
    FileSystemException::raise(this, FS_READ_FAILED_PATH_DOESNT_EXIST, path);
}
