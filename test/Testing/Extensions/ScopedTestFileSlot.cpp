#include "ScopedTestFileSlot.h"

#include <filesystem>

ScopedTestFileSlot::ScopedTestFileSlot(const NativePath &path) : _path(path) {
    std::error_code ec;
    std::filesystem::remove(_path.toStdPath(), ec);
}

ScopedTestFileSlot::~ScopedTestFileSlot() {
    std::error_code ec;
    std::filesystem::remove(_path.toStdPath(), ec);
}
