#include "NormalPath.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <utility>

NormalPath::NormalPath(PathView path) {
    std::string copy(path.str());
    std::ranges::replace(copy, '\\', '/');
    while (copy.starts_with("//"))
        copy.erase(0, 1); // On Windows this would be a network root.

    _path = Path(std::move(copy));
    if (!_path.isNormalized())
        _path = _path.normalized();
}

NormalPath &NormalPath::operator/=(NormalPathView tail) {
    *this = NormalPathView(_path) / tail;
    return *this;
}

NormalPath operator/(NormalPathView head, NormalPathView tail) {
    assert(head.isAccessible() && tail.isAccessible());

    if (tail.isRoot() || head.isRoot()) {
        NormalPath path;
        path._path = Path(PathView(tail.isRoot() ? head : tail));
        return path;
    }

    std::string_view headString = head.str();
    std::string_view tailString = tail.str();

    std::string result;
    result.reserve(headString.size() + tailString.size());
    result += headString;
    result += tailString;

    NormalPath path;
    path._path = Path(std::move(result));
    return path;
}
