#include "Path.h"

#include <algorithm>
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

#ifdef _WINDOWS
std::wstring Path::native() const {
    std::wstring result = txt::wtf8ToWide(_path);

    // Win32 only recognizes a literal "\\?\". Spelled with forward slashes the prefix gets parsed like any other path,
    // which brings back MAX_PATH and strips a trailing dot off a file name. A "//./" path gets parsed whichever slashes
    // it uses, so converting it too does no harm.
    using detail::isPathSeparator;
    bool isExtended = _path.size() >= 4 && isPathSeparator(_path[0]) && isPathSeparator(_path[1]) &&
                      (_path[2] == '?' || _path[2] == '.') && isPathSeparator(_path[3]);
    if (isExtended)
        std::ranges::replace(result, L'/', L'\\');

    return result;
}
#endif
