#include "ScopedTestFileSlot.h"

#include <filesystem>

ScopedTestFileSlot::ScopedTestFileSlot(std::string_view path) : _path(path) {
    std::error_code ec;
    std::filesystem::remove(_path, ec);
}

ScopedTestFileSlot::~ScopedTestFileSlot() {
    std::error_code ec;
    std::filesystem::remove(_path, ec);
}
