#include "FileSystemPathView.h"

#include <cassert>
#include <ranges>

bool FileSystemPathView::isParentOf(FileSystemPathView child) const {
    if (isEscaping()) {
        if (child.isEscaping()) {
            // Both paths are escaping. We need to skip the common prefix first.
            auto mismatch = std::ranges::mismatch(split(), child.split());
            FileSystemPathView parentTail = split().tailAt(mismatch.in1);
            FileSystemPathView childTail = child.split().tailAt(mismatch.in2);
            assert(!parentTail.isEscaping() || !childTail.isEscaping());
            return parentTail.isParentOf(childTail);
        } else {
            // Escaping path can be a parent of a non-escaping path only if it has a form ../../../..
            return _path == ".." || _path.ends_with("/..");
        }
    } else {
        if (child.isEscaping()) {
            // Non-escaping path can never be a parent of an escaping path.
            return false;
        } else {
            // Both paths are non-escaping, child must have at least one more chunk than the parent.
            return child._path.size() > _path.size() && child._path.starts_with(_path) && (child._path[_path.size()] == '/' || _path.empty());
        }
    }
}
