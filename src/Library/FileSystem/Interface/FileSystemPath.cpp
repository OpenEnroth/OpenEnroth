#include "FileSystemPath.h"

#include <string>
#include <string_view>

#include "Utility/String/Split.h"
#include "Utility/String/Join.h"
#include "Utility/String/Transformations.h"
#include "Utility/SmallVector.h"

// Offset of the file name inside the path, so `path.substr(fileNameOffset(path))` is the last component.
static size_t fileNameOffset(std::string_view path) {
    size_t separatorPos = path.rfind('/');
    return separatorPos == std::string_view::npos ? 0 : separatorPos + 1;
}

// Offset of the extension inside the path, or npos if there's none. Mirrors std::filesystem: a leading dot doesn't
// start an extension, so ".bashrc" has none, and ".." has none either. Normal form never spells a name as ".".
static size_t extensionOffset(std::string_view path) {
    size_t nameOffset = fileNameOffset(path);
    std::string_view fileName = path.substr(nameOffset);
    if (fileName == "..")
        return std::string_view::npos;

    size_t dotPos = fileName.rfind('.');
    if (dotPos == std::string_view::npos || dotPos == 0)
        return std::string_view::npos;

    return nameOffset + dotPos;
}

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

std::string_view FileSystemPath::name() const {
    return std::string_view(_path).substr(fileNameOffset(_path));
}

std::string_view FileSystemPath::extension() const {
    size_t offset = extensionOffset(_path);
    return offset == std::string::npos ? std::string_view() : std::string_view(_path).substr(offset);
}

std::string_view FileSystemPath::stem() const {
    size_t offset = extensionOffset(_path);
    std::string_view name = this->name();
    return offset == std::string::npos ? name : name.substr(0, name.size() - (_path.size() - offset));
}

FileSystemPath FileSystemPath::parent() const {
    size_t end = fileNameOffset(_path);
    if (end > 0)
        end--; // Normal form has single separators, so this drops the one between the parent and the file name.

    return fromNormalized(_path.substr(0, end));
}

FileSystemPath &FileSystemPath::operator/=(std::string_view tail) {
    // Please no '\\' path separators.
    std::string tmp;
    if (tail.contains('\\')) {
        tmp = replaceAll(tail, '\\', '/');
        tail = tmp;
    }

    gch::small_vector<std::string_view, 32> stack;
    for (std::string_view chunk : ::split(tail).by('/')) {
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
