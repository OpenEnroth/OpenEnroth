#include "NormalPath.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/String/Ascii.h"

bool NormalPathView::isAccessible() const {
    std::string_view path = str();
    if (path.empty() || !_path.root().empty())
        return false;
    if (path == ".." || path.starts_with("../"))
        return false; // Normal form keeps every ".." at the start.

#ifdef _WINDOWS
    for (std::string_view segment : split())
        if (segment.size() >= 2 && segment[1] == ':' && (ascii::isLower(segment[0]) || ascii::isUpper(segment[0])))
            return false;
#endif
    return true;
}

NormalPath::NormalPath(PathView path) {
    if (path.str().contains('\\')) {
        std::string copy(path.str());
        std::ranges::replace(copy, '\\', '/');
        _path = Path(std::move(copy));
    } else {
        _path = Path(path);
    }

    if (!_path.isEmpty() && !_path.isNormalized())
        _path = _path.normalized();
}

NormalPath &NormalPath::operator/=(NormalPathView tail) {
    if (NormalPathView(_path).isRoot()) {
        _path = Path(PathView(tail));
    } else if (!tail.isRoot()) {
        _path /= PathView(tail);
    }
    return *this;
}

NormalPath operator/(NormalPathView head, NormalPathView tail) {
    std::string_view headString = head.relativeStr();
    std::string_view tailString = tail.relativeStr();

    if (headString.empty() || tailString.empty()) {
        NormalPath path;
        path._path = Path(PathView(headString.empty() ? tail : head));
        return path;
    }

    std::string result;
    result.reserve(headString.size() + 1 + tailString.size());
    result += headString;
    result += '/';
    result += tailString;

    NormalPath path;
    path._path = Path(std::move(result));
    return path;
}
