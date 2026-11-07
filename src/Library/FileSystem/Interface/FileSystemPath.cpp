#include "FileSystemPath.h"

#include <string>
#include <string_view>

#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

static size_t fileNameOffset(std::string_view path) {
    return path.rfind('/') + 1; // Normal form always starts with a separator.
}

/**
 * @param path                          Path in normal form, so a name is never spelled `"."` or `".."`.
 * @return                              Offset of the extension inside the path, or `npos` if there is none. Mirrors
 *                                      `std::filesystem`, so a leading dot doesn't start an extension and
 *                                      `".bashrc"` has none.
 */
static size_t extensionOffset(std::string_view path) {
    size_t nameOffset = fileNameOffset(path);
    std::string_view fileName = path.substr(nameOffset);

    size_t dotPos = fileName.rfind('.');
    if (dotPos == std::string_view::npos || dotPos == 0)
        return std::string_view::npos;

    return nameOffset + dotPos;
}

/**
 * @param[in,out] path                  Path in normal form, loses its last segment. The root stays the root.
 */
static void popOneChunk(std::string &path) {
    size_t separatorPos = path.rfind('/');
    path.resize(separatorPos == 0 ? 1 : separatorPos);
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
    FileSystemPath result = *this;
    popOneChunk(result._path);
    return result;
}

FileSystemPath &FileSystemPath::operator/=(std::string_view tail) {
    // Please no '\\' path separators.
    std::string tmp;
    if (tail.contains('\\')) {
        tmp = replaceAll(tail, '\\', '/');
        tail = tmp;
    }

    for (std::string_view chunk : ::split(tail).by('/')) {
        if (chunk.empty() || chunk == ".")
            continue;

        if (chunk == "..") {
            popOneChunk(_path); // Stops at the root.
            continue;
        }

        if (!isRoot())
            _path += '/';
        _path += chunk;
    }

    return *this;
}

FileSystemPath &FileSystemPath::operator/=(FileSystemPathView tail) {
    if (isRoot()) {
        _path = tail.string();
    } else if (!tail.isRoot()) {
        _path += tail.string();
    }
    return *this;
}
