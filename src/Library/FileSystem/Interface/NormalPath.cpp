#include "NormalPath.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <string_view>
#include <utility>

bool NormalPathView::isAccessible() const {
    if (!_path.root().empty())
        return false;

#ifdef _WINDOWS
    for (std::string_view segment : split())
        if (!PathView(segment).root().empty())
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
        rest.remove_prefix(rest.size() > 2 ? 3 : 2); // Path::normalized leaves every ".." at the start, and ".." stops at the root.
    if (rest == ".")
        rest = {};
    if (rest.size() != _path.str().size()) {
        _path = Path(std::string(rest));
        if (!_path.isNormalized())
            _path = _path.normalized(); // On Windows dropping the ".." can expose a drive, as in "../C:.".
    }
}

NormalPath &NormalPath::operator/=(NormalPathView tail) {
    assert(isAccessible() && tail.isAccessible());

    if (!tail.isEmpty()) // Path gives "a/" for "a" / "".
        _path /= PathView(tail);
    return *this;
}

NormalPath operator/(NormalPathView head, NormalPathView tail) {
    assert(head.isAccessible() && tail.isAccessible());

    NormalPath path;
    path._path = tail.isEmpty() ? Path(PathView(head)) : PathView(head) / PathView(tail); // Path gives "a/" for "a" / "".
    return path;
}
