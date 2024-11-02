#include "FileSystemPathView.h"

bool FileSystemPathView::isParentOf(const FileSystemPathView &child) const {
    if (isEmpty())
        return true; // Root is a parent of everything, including itself.
    return child._path.size() > _path.size() && child._path.starts_with(_path) && child._path[_path.size()] == '/';
}

