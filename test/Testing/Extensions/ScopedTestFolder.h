#pragma once

#include "Utility/System/Path.h"

/**
 * Helper class to create a temporary folder at the given path & remove it, with everything that's in it, when leaving
 * the current scope.
 */
class ScopedTestFolder {
 public:
    explicit ScopedTestFolder(const Path &path);
    ~ScopedTestFolder();

 private:
    Path _path;
};
