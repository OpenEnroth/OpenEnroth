#include "NormalizedFileSystemPath.h"

#include <algorithm>
#include <string>
#include <string_view>

NormalizedFileSystemPath::NormalizedFileSystemPath(PathView path) : _view(path) {
    std::string_view string = path.str();
    if (!string.contains('\\') && !string.starts_with('/') && path.isNormalized())
        return;

    std::string copy(string);
    std::ranges::replace(copy, '\\', '/');
    copy.erase(0, copy.find_first_not_of('/'));
    _owned = Path(copy).normalized();
    _view = _owned;
}
