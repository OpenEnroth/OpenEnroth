#include "Path.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/SmallVector.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Encoding.h"
#include "Utility/String/Split.h"

// Everything below operates on the stored string, where the only separator is a forward slash.
static constexpr char separator = '/';

// Whether the path starts with a drive letter followed by a root directory, e.g. "C:/". A bare "C:" is not root
// syntax - there is no cross-platform meaning to preserve, so it parses as ordinary relative bytes.
static bool hasDriveRoot([[maybe_unused]] std::string_view path) {
#ifdef _WINDOWS
    return path.size() >= 3 && (ascii::isLower(path[0]) || ascii::isUpper(path[0])) && path[1] == ':' &&
           path[2] == separator;
#else
    return false;
#endif
}

// Length of the root prefix - the leading part that normalization preserves verbatim. That's "" or "/" everywhere,
// plus "//" on POSIX (a path starting with exactly two slashes is implementation-defined, so we don't touch it), and
// "C:/" or "//server" / "//server/" on Windows. The root is what a relative path is relative to, so an absolute path
// is exactly one that has a non-empty root.
static size_t rootSize(std::string_view path) {
    if (hasDriveRoot(path))
        return 3;

    size_t slashes = 0;
    while (slashes < path.size() && path[slashes] == separator)
        slashes++;

#ifdef _WINDOWS
    if (slashes == 2 && path.size() > 2) {
        size_t shareEnd = std::min(path.find(separator, 2), path.size()); // A UNC share, "//server" in "//server/x".
        return shareEnd < path.size() ? shareEnd + 1 : shareEnd;
    }
#else
    if (slashes == 2)
        return 2;
#endif

    return slashes > 0 ? 1 : 0;
}

// Offset of the file name inside the path, so `path.substr(fileNameOffset(path))` is the last component.
static size_t fileNameOffset(std::string_view path) {
    size_t separatorPos = path.rfind(separator);
    return std::max(separatorPos == std::string_view::npos ? 0 : separatorPos + 1, rootSize(path));
}

// Offset of the extension inside the path, or npos if there's none. A leading dot doesn't start an extension, so
// ".bashrc" has no extension, and neither do "." and "..".
static size_t extensionOffset(std::string_view path) {
    size_t nameOffset = fileNameOffset(path);
    std::string_view fileName = path.substr(nameOffset);
    if (fileName == "." || fileName == "..")
        return std::string_view::npos;

    size_t dotPos = fileName.rfind('.');
    if (dotPos == std::string_view::npos || dotPos == 0)
        return std::string_view::npos;

    return nameOffset + dotPos;
}

// Rewrites the path into the lexical normal form - see the class docs for what that is.
static std::string normalized(std::string_view path) {
    std::string result;

#ifdef _WINDOWS
    std::string converted(path);
    std::ranges::replace(converted, '\\', separator); // Both slashes separate components on Windows.
    path = converted;
#endif

    size_t root = rootSize(path);
    bool isRooted = root > 0;
    result = path.substr(0, root);
    if (isRooted && result.back() != separator)
        result += separator; // A bare share name is a root, and a root carries its separator - "//server" is "//server/".

    gch::small_vector<std::string_view, 32> stack;
    for (std::string_view chunk : split(path.substr(root)).by(separator)) {
        if (chunk.empty() || chunk == ".")
            continue;

        if (chunk == "..") {
            if (!stack.empty() && stack.back() != "..") {
                stack.pop_back();
                continue;
            }
            if (isRooted)
                continue; // Above an absolute root ".." is the root itself, same as POSIX defines it.
        }

        stack.push_back(chunk);
    }

    for (size_t i = 0; i < stack.size(); i++) {
        if (i > 0 || (!result.empty() && result.back() != separator))
            result += separator;
        result += stack[i];
    }

    return result;
}

Path::Path(std::string_view path) : _path(normalized(path)) {}

#ifdef _WINDOWS
Path Path::fromNative(std::wstring_view path) {
    return Path(txt::wideToWtf8(path));
}
#else
Path Path::fromNative(std::string_view path) {
    return Path(path);
}
#endif

#ifdef _WINDOWS
std::wstring Path::native() const {
    return txt::wtf8ToWide(_path); // Win32 takes forward slashes just fine, no need to convert them back.
}
#endif

std::string Path::displayString() const {
    return txt::encodedToUtf8(_path, ENCODING_UTF8); // UTF-8 to UTF-8 conversion replaces all the invalid parts.
}

std::string_view Path::root() const {
    return std::string_view(_path).substr(0, rootSize(_path));
}

bool Path::isEscaping() const {
    return _path == ".." || _path.starts_with("../");
}

std::string_view Path::name() const {
    return std::string_view(_path).substr(fileNameOffset(_path));
}

std::string_view Path::extension() const {
    size_t offset = extensionOffset(_path);
    return offset == std::string::npos ? std::string_view() : std::string_view(_path).substr(offset);
}

std::string_view Path::stem() const {
    size_t offset = extensionOffset(_path);
    std::string_view name = this->name();
    return offset == std::string::npos ? name : name.substr(0, name.size() - (_path.size() - offset));
}

Path Path::parent() const {
    size_t root = rootSize(_path);
    size_t end = fileNameOffset(_path);

    // Drop the separators between the parent and the file name, but keep the one that is the root directory.
    while (end > root && _path[end - 1] == separator)
        end--;

    return fromNormalized(_path.substr(0, end));
}

Path Path::withExtension(std::string_view extension) const {
    assert(!extension.contains(separator)); // An extension is not a path.

    size_t offset = extensionOffset(_path);
    std::string result = offset == std::string::npos ? _path : _path.substr(0, offset);

    if (!extension.empty()) {
        if (extension[0] != '.')
            result += '.';
        result += extension;
    }

    // Truncating at the last dot can leave a "." or ".." file name - "a/..b" without an extension is "a/.", which
    // is "a". So the result goes back through normalization instead of being taken on trust.
    return Path(result);
}

Path Path::operator/(const Path &tail) const {
    if (tail.isAbsolute())
        return tail; // A rooted tail replaces the head, same as std::filesystem::path::operator/ does it.

    if (_path.empty())
        return tail;
    if (tail._path.empty())
        return *this;

    // Re-normalizing the seam is what collapses a leading ".." run in the tail against the head.
    std::string result = _path;
    if (result.back() != separator)
        result += separator;
    result += tail._path;
    return Path(result);
}
