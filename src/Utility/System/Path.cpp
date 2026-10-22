#include "Path.h"

#include <string>
#include <string_view>

#include "Utility/String/Encoding.h"

#ifdef _WINDOWS
Path Path::fromNative(std::wstring_view path) {
    return Path(txt::wideToWtf8(path));
}
#else
Path Path::fromNative(std::string_view path) {
    return Path(path);
}
#endif

Path &Path::operator/=(PathView tail) {
    if (!tail.root().empty())
        return *this = PathView(*this) / tail;

    size_t size = _path.size();
    _path += tail.str(); // Appending takes a source inside the string's own buffer, so the tail may point into it.
    if (size > 0 && !detail::isPathSeparator(_path[size - 1]))
        _path.insert(size, 1, '/'); // After a bare drive letter too, as in operator/.
    return *this;
}
