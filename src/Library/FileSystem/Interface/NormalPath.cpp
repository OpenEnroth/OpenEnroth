#include "NormalPath.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <string_view>
#include <utility>

#include "Utility/String/Ascii.h"

bool NormalPathView::isAccessible() const {
    if (!_path.root().empty())
        return false;

#ifdef _WINDOWS
    for (std::string_view segment : split())
        if (segment.size() >= 2 && segment[1] == ':' && (ascii::isLower(segment[0]) || ascii::isUpper(segment[0])))
            return false;
#endif
    return true;
}

NormalPath::NormalPath(PathView path) {
    std::string copy(path.str());
    std::ranges::replace(copy, '\\', '/');
    _path = Path(std::move(copy));
    if (!_path.isNormalized())
        _path = _path.normalized();

    if (!_path.root().empty())
        return;

    std::string_view rest = _path.str();
    while (rest == ".." || rest.starts_with("../"))
        rest.remove_prefix(rest.size() > 2 ? 3 : 2); // Normal form keeps every ".." at the start, and ".." stops at the root.
    if (rest == ".")
        rest = {};
    if (rest.size() != _path.str().size())
        _path = Path(std::string(rest));
}

NormalPath &NormalPath::operator/=(NormalPathView tail) {
    assert(isAccessible() && tail.isAccessible());

    if (_path.isEmpty()) {
        _path = Path(PathView(tail));
    } else if (!tail.isRoot()) {
        _path /= PathView(tail);
    }
    return *this;
}

NormalPath operator/(NormalPathView head, NormalPathView tail) {
    assert(head.isAccessible() && tail.isAccessible());

    if (head.isRoot() || tail.isRoot()) {
        NormalPath path;
        path._path = Path(PathView(head.isRoot() ? tail : head));
        return path;
    }

    std::string_view headString = head.str();
    std::string_view tailString = tail.str();

    std::string result;
    result.reserve(headString.size() + 1 + tailString.size());
    result += headString;
    result += '/';
    result += tailString;

    NormalPath path;
    path._path = Path(std::move(result));
    return path;
}
