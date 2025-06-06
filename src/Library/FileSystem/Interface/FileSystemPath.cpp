#include "FileSystemPath.h"

#include <string>

#include "Utility/String/Split.h"
#include "Utility/String/Join.h"
#include "Utility/String/Transformations.h"
#include "Utility/SmallVector.h"

template<class String>
inline bool popOneChunk(String &path) {
    if (path.empty())
        return false;

    size_t splitPos = path.find_last_of('/');
    size_t chunkPos;
    if (splitPos == std::string_view::npos) {
        splitPos = 0;
        chunkPos = 0;
    } else {
        chunkPos = splitPos + 1;
    }

    if (std::string_view(path).substr(chunkPos) == "..")
        return false;

    path.resize(splitPos);
    return true;
}

FileSystemPath::FileSystemPath(std::string_view path) {
    operator/=(path);
}

FileSystemPath &FileSystemPath::operator/=(std::string_view tail) {
    // Please no '\\' path separators.
    std::string tmp;
    if (tail.contains('\\')) {
        tmp = replaceAll(tail, '\\', '/');
        tail = tmp;
    }

    gch::small_vector<std::string_view, 32> stack;
    for (std::string_view chunk : ::split(tail, '/')) {
        if (chunk.empty())
            continue;

        if (chunk == ".")
            continue;

        if (chunk == "..") {
            if (!stack.empty()) {
                if (stack.back() != "..") {
                    stack.pop_back();
                    continue;
                }
            } else if (popOneChunk(_path)) {
                continue;
            }
        }

        stack.push_back(chunk);
    }

    if (_path.empty()) {
        _path = join(stack, '/');
    } else {
        for (std::string_view chunk : stack) {
            _path += '/';
            _path += chunk;
        }
    }

    return *this;
}

FileSystemPath &FileSystemPath::operator/=(FileSystemPathView tail) {
    std::string_view tailsTail = tail.string();

    if (tail.isEscaping()) {
        while (true) {
            if (!popOneChunk(_path))
                break;

            tailsTail = tailsTail.substr(tailsTail.size() > 2 ? 3 : 2); // Skip "../" that we've just processed.
            if (tailsTail != ".." && !tailsTail.starts_with("../"))
                break;
        }
    }

    if (!_path.empty() && !tailsTail.empty())
        _path += '/';
    _path += tailsTail;

    return *this;
}
