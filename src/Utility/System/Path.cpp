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
// "C:/", "//server/" or an extended-length prefix like "//?/C:/" on Windows. The root is what a relative path is relative to, so an absolute path
// is exactly one that has a non-empty root.
static size_t rootSize(std::string_view path) {
    if (hasDriveRoot(path))
        return 3;

    size_t slashes = 0;
    while (slashes < path.size() && path[slashes] == separator)
        slashes++;

#ifdef _WINDOWS
    if (slashes == 2 && path.size() > 2) {
        // An extended-length path, "//?/C:/x" or "//./COM1". Win32 does no parsing on these - that's the point of
        // the prefix - so the volume or device that follows it anchors the path the same way a drive letter does,
        // and "//?/UNC/server/share" spells a share with two components instead of one.
        bool isExtended = path.compare(2, 2, "?/") == 0 || path.compare(2, 2, "./") == 0;
        size_t components = 1;
        if (isExtended)
            components = path.compare(4, 4, "UNC/") == 0 ? 4 : 2;

        size_t pos = 2; // The first character after the leading "//".
        for (size_t i = 0; i < components; i++) {
            size_t next = std::min(path.find(separator, pos), path.size());
            std::string_view component = path.substr(pos, next - pos);

            // A root is copied through normalization verbatim, so nothing that needs normalizing can be part of
            // one - otherwise "//?/../x" would keep its dots all the way to Win32, which doesn't resolve them. The
            // "?" or "." that opens an extended-length path is exempt, being the prefix rather than a component.
            if (component.empty() || (i > 0 && (component == "." || component == "..")))
                return 1;

            if (next >= path.size())
                return path.size(); // The root is the whole path, with no separator to carry.

            pos = next + 1;
        }

        return pos; // Just past the separator, which the root carries.
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

// Offset of the extension inside the path, or npos if there's none. Mirrors std::filesystem: a leading dot doesn't
// start an extension, so ".bashrc" has none, and the names "." and ".." have none either.
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

// Rewrites the path into the lexical normal form - see Path::normalized for what that is. Separators are already
// forward slashes here, the constructor does that.
static std::string normalized(std::string_view path) {
    std::string result;

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

Path::Path(std::string_view path) : _path(path) {
#ifdef _WINDOWS
    std::ranges::replace(_path, '\\', separator); // Both slashes separate components on Windows.
#endif
}

bool Path::isNormalizedImpl(std::string_view path) {
    size_t root = rootSize(path);
    if (root > 0 && path[root - 1] != separator)
        return false; // A root carries its separator, so a bare share name isn't normal.

    std::string_view tail = path.substr(root);
    if (tail.empty())
        return true;
    if (tail.back() == separator)
        return false; // No trailing separator.

    bool leading = true;
    for (std::string_view chunk : ::split(tail).by(separator)) {
        if (chunk.empty() || chunk == ".")
            return false;

        if (chunk == "..") {
            if (root > 0 || !leading)
                return false; // ".." survives only as the leading run of a relative path.
        } else {
            leading = false;
        }
    }

    return true;
}

Path Path::normalized() const {
    return fromNormalized(::normalized(_path));
}

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
    std::wstring result = txt::wtf8ToWide(_path);

    // Win32 takes forward slashes everywhere except in an extended-length path, where it does no parsing at all and
    // a forward slash is just a character a file name can't contain.
    if (result.starts_with(L"//?/") || result.starts_with(L"//./"))
        std::ranges::replace(result, L'/', L'\\');

    return result;
}
#endif

std::string Path::displayString() const {
    return txt::encodedToUtf8(_path, ENCODING_UTF8); // UTF-8 to UTF-8 conversion replaces all the invalid parts.
}

std::string_view Path::rootOf(std::string_view path) {
    return path.substr(0, rootSize(path));
}

bool Path::isEscapingImpl(std::string_view path) {
    if (rootSize(path) > 0)
        return false; // An absolute path can't escape, ".." is clamped by the root.

    int depth = 0;
    for (std::string_view chunk : ::split(path).by(separator)) {
        if (chunk.empty() || chunk == ".")
            continue;

        if (chunk == "..") {
            if (--depth < 0)
                return true;
        } else {
            depth++;
        }
    }

    return false;
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

bool Path::isExtension(std::string_view extension) {
    if (extension.empty())
        return true; // Dropping the extension.
    if (extension[0] != '.')
        return false;
    if (extension.size() > 1 && extension.back() == '.')
        return false; // "..", "...", ".txt." - Windows trims a trailing dot and POSIX doesn't, so the same literal
                      // would name two different files.

    static constexpr std::string_view forbidden("/\\\0", 3); // Explicit length, so the NUL is part of the needle.
    return extension.find_first_of(forbidden) == std::string_view::npos;
}

Path Path::withExtension(std::string_view extension) const {
    assert(isExtension(extension));

    size_t offset = extensionOffset(_path);
    std::string result = offset == std::string::npos ? _path : _path.substr(0, offset);

    if (!extension.empty()) {
        if (extension[0] != '.')
            result += '.';
        result += extension;
    }

    return fromNormalized(std::move(result));
}

Path Path::operator/(const Path &tail) const {
    if (tail.isAbsolute())
        return tail; // A rooted tail replaces the head, same as std::filesystem::path::operator/ does it.

    if (_path.empty())
        return tail;
    if (tail._path.empty())
        return *this;

    std::string result = _path;
    if (result.back() != separator)
        result += separator;
    result += tail._path;
    return fromNormalized(std::move(result)); // Plain concatenation - normalizing the seam is the caller's call.
}
