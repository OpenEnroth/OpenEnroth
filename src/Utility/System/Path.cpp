#include "Path.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/String/Encoding.h"

Path::Path(std::string_view path) : Path(std::string(path)) {}

Path::Path(std::string &&path) : _path(std::move(path)) {
#ifdef _WINDOWS
    std::ranges::replace(_path, '\\', '/'); // Both slashes separate components on Windows.
#endif
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

    // Win32 only recognizes a literal "\\?\". Spelled with forward slashes the prefix gets parsed like any other path,
    // which brings back MAX_PATH and strips a trailing dot off a file name. A "//./" path gets parsed whichever slashes
    // it uses, so converting it too does no harm.
    if (_path.starts_with("//?/") || _path.starts_with("//./"))
        std::ranges::replace(result, L'/', L'\\');

    return result;
}
#endif
