#pragma once

#include "Utility/String/AsciiLiteral.h"
#include "Utility/System/NativePath.h"

/**
 * Helper class to remove a file in ctor and when leaving the current scope.
 *
 * Essentially a counterpart to `ScopedTestFile`. While the latter creates a file, this class makes sure a 'slot' exist
 * where the user can create a file.
 */
class ScopedTestFileSlot {
 public:
    explicit ScopedTestFileSlot(AsciiLiteral path) : ScopedTestFileSlot(NativePath(path)) {}
    explicit ScopedTestFileSlot(const NativePath &path);
    ~ScopedTestFileSlot();

 private:
    NativePath _path;
};
