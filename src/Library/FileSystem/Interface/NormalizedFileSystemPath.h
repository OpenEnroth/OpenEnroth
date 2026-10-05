#pragma once

#include "Utility/System/Path.h"

/**
 * A path inside a `FileSystem`, brought to the normal form the file system layer works in. A backslash is a
 * separator on every platform, leading separators are dropped, so `"/a"` is `"a"`, and the rest is normalized. A
 * path that's in normal form already is viewed rather than copied.
 */
class NormalizedFileSystemPath {
 public:
    explicit NormalizedFileSystemPath(PathView path);

    NormalizedFileSystemPath(const NormalizedFileSystemPath &) = delete; // The view can point into _owned.
    NormalizedFileSystemPath &operator=(const NormalizedFileSystemPath &) = delete;

    operator PathView() const { // NOLINT: intentionally implicit.
        return _view;
    }

    [[nodiscard]] bool isEmpty() const {
        return _view.isEmpty();
    }

    /**
     * @return                          Whether a file system can reach this path. It can't if the path escapes the
     *                                  root, or names a drive on Windows, as `"C:/a"` and `"./C:/a"` do.
     */
    [[nodiscard]] bool isAccessible() const {
        return _view.root().empty() && !_view.isEscaping();
    }

 private:
    Path _owned;
    PathView _view;
};
