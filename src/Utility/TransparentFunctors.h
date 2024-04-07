#pragma once

#include <functional> // For std::hash, std::equal_to, std::less.
#include <string_view>

struct TransparentStringHash : std::hash<std::string_view> {
    using is_transparent = void;
};

struct TransparentStringEquals : std::equal_to<std::string_view> {
    using is_transparent = void;
};

struct TransparentStringLess : std::less<std::string_view> {
    using is_transparent = void;
};
