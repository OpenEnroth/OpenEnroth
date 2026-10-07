#pragma once

#include "Utility/System/Path.h"

/**
 * A path inside a `FileSystem`, brought to the normal form the file system layer works in. A backslash is a
 * separator on every platform, and the rest is normalized. A path that's in normal form already is viewed rather
 * than copied, so it has to outlive this object.
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
     * @return                          Whether a file system can reach this path. It can't if the path has a root,
     *                                  as `"/a"` does, or escapes the file system's root. On Windows it can't if a
     *                                  segment starts with a drive, as in `"a/c:b"`, because a split hands out tails
     *                                  that start at a segment, and such a tail reads as a root.
     */
    [[nodiscard]] bool isAccessible() const;

 private:
    Path _owned;
    PathView _view;
};
