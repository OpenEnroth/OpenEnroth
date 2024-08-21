#pragma once

#include <string_view>
#include <string>

/**
 * Helper class to remove a file in ctor and when leaving the current scope.
 *
 * Essentially a counterpart to `ScopedTestFile`. While the latter creates a file, this class makes sure a 'slot' exist
 * where the user can create a file.
 */
class ScopedTestFileSlot {
 public:
    explicit ScopedTestFileSlot(std::string_view path);
    ~ScopedTestFileSlot();

 private:
    std::string _path;
};
