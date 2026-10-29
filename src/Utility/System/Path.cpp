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
        return *this = detail::joinPaths(*this, tail);

    size_t headSize = detail::joinedHeadSize(*this);
    _path.replace(headSize, std::string::npos, tail.str()); // The tail may point into _path, replace() takes that.
    if (headSize > 0 && !detail::isPathSeparator(_path[headSize - 1]))
        _path.insert(headSize, 1, '/'); // After a bare drive letter too, as in operator/.
    return *this;
}
