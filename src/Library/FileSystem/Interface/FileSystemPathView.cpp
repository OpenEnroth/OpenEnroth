#include "FileSystemPathView.h"

#include <cassert>

bool FileSystemPathView::isParentOf(FileSystemPathView child) const {
    if (isEscaping()) {
        if (child.isEscaping()) {
            // Both paths are escaping. We need to skip the common prefix first.
            auto parentChunks = chunks();
            auto childChunks = child.chunks();

            // TODO(captainurist): this begs for a proper API at chunks() level so that we could use std::mismatch.
            auto parentPos = parentChunks.begin();
            auto parentEnd = parentChunks.end();
            auto childPos = childChunks.begin();
            auto childEnd = childChunks.end();
            assert(parentPos != parentEnd && childPos != childEnd);

            std::string_view lastParentChunk, lastChildChunk;
            do  {
                std::string_view parentChunk = *parentPos;
                std::string_view childChunk = *childPos;
                if (parentChunk != childChunk)
                    break;

                lastParentChunk = parentChunk;
                lastChildChunk = childChunk;
                ++parentPos;
                ++childPos;
            } while (parentPos != parentEnd && childPos != childEnd);

            // Now at least one of the paths is non-escaping.
            FileSystemPathView parentTail = tailAfter(lastParentChunk);
            FileSystemPathView childTail = child.tailAfter(lastChildChunk);
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
