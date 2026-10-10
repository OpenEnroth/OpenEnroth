#include "NormalPath.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/String/Ascii.h"

NormalPath::NormalPath(std::string_view path) {
    if (path.contains('\\')) {
        std::string copy(path);
        std::ranges::replace(copy, '\\', '/');
        _path = Path(std::move(copy));
    } else {
        _path = Path(path);
    }

    if (!_path.isNormalized())
        _path = _path.normalized();
    if (_path.str() == ".")
        _path = Path(); // The file system's root is spelled "".
}

bool NormalPath::isAccessible() const {
    if (!_path.root().empty() || _path.isEscaping())
        return false;

#ifdef _WINDOWS
    for (std::string_view segment : _path.split())
        if (segment.size() >= 2 && segment[1] == ':' && (ascii::isLower(segment[0]) || ascii::isUpper(segment[0])))
            return false;
#endif
    return true;
}

NormalPath &NormalPath::operator/=(NormalPathView tail) {
    if (!tail.isEmpty())
        _path /= PathView(tail); // An empty tail would leave a trailing separator.
    return *this;
}

NormalPath operator/(NormalPathView head, NormalPathView tail) {
    std::string_view headString = head.str();
    std::string_view tailString = tail.str();

    std::string result;
    result.reserve(headString.size() + 1 + tailString.size());
    result += headString;
    if (!headString.empty() && !tailString.empty())
        result += '/';
    result += tailString;

    NormalPath path;
    path._path = Path(std::move(result));
    return path;
}
