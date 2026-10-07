#include "NormalizedFileSystemPath.h"

#include <algorithm>
#include <string>
#include <string_view>

#include "Utility/String/Ascii.h"

NormalizedFileSystemPath::NormalizedFileSystemPath(PathView path) : _view(path) {
    std::string_view str = path.str();
    if (!str.contains('\\') && path.isNormalized())
        return;

    std::string copy(str);
    std::ranges::replace(copy, '\\', '/');
    _owned = Path(copy).normalized();
    _view = _owned;
}

bool NormalizedFileSystemPath::isAccessible() const {
    if (!_view.root().empty() || _view.isEscaping())
        return false;

#ifdef _WINDOWS
    for (std::string_view segment : _view.split())
        if (segment.size() >= 2 && segment[1] == ':' && (ascii::isLower(segment[0]) || ascii::isUpper(segment[0])))
            return false;
#endif
    return true;
}
