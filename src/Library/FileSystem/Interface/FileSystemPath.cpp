#include "FileSystemPath.h"

#include <cassert>
#include <string>
#include <utility>

#include "Utility/String/Split.h"
#include "Utility/String/Join.h"
#include "Utility/String/Transformations.h"
#include "Utility/SmallVector.h"

FileSystemPath::FileSystemPath(std::string_view path) : _path(normalizePath(path)) {}

FileSystemPath FileSystemPath::fromNormalized(std::string path) {
    assert(normalizePath(path) == path);

    FileSystemPath result;
    result._path = std::move(path);
    return result;
}

std::string FileSystemPath::normalizePath(std::string_view path) {
    std::string normalPath = replaceAll(path, '\\', '/'); // Please no '\\' path separators.

    gch::small_vector<std::string_view, 32> stack;
    for (std::string_view chunk : split(normalPath, '/')) {
        if (chunk.empty())
            continue;

        if (chunk == ".")
            continue;

        if (chunk == "..") {
            if (!stack.empty())
                stack.pop_back();
            continue;
        }

        stack.push_back(chunk);
    }

    return join(stack, '/');
}
