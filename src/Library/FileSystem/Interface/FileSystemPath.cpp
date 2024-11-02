#include "FileSystemPath.h"

#include <string>

#include "Utility/String/Split.h"
#include "Utility/String/Join.h"
#include "Utility/String/Transformations.h"
#include "Utility/SmallVector.h"

FileSystemPath::FileSystemPath(std::string_view path) : _path(normalizePath(path)) {}

void FileSystemPath::append(FileSystemPathView tail) {
    std::string_view tailsTail = tail.string();

    if (tail.isEscaping()) {
        while (true) {
            if (_path.empty())
                break;

            size_t splitPos = _path.find_last_of('/');
            size_t chunkPos;
            if (splitPos == std::string_view::npos) {
                splitPos = 0;
                chunkPos = 0;
            } else {
                chunkPos = splitPos + 1;
            }

            if (std::string_view(_path).substr(chunkPos) == "..")
                break;

            _path.resize(splitPos);
            tailsTail = tailsTail.substr(tailsTail.size() > 2 ? 3 : 2); // Skip "../" that we've just processed.
            if (tailsTail != ".." && !tailsTail.starts_with("../"))
                break;
        }
    }

    if (!_path.empty() && !tailsTail.empty())
        _path += '/';
    _path += tailsTail;
}

std::string FileSystemPath::normalizePath(std::string_view path) {
    std::string normalPath = replaceAll(path, '\\', '/'); // Please no '\\' path separators.

    gch::small_vector<std::string_view, 32> stack;
    for (std::string_view chunk : ::split(normalPath, '/')) {
        if (chunk.empty())
            continue;

        if (chunk == ".")
            continue;

        if (chunk == ".." && !stack.empty() && stack.back() != "..") {
            stack.pop_back();
            continue;
        }

        stack.push_back(chunk);
    }

    return join(stack, '/');
}
