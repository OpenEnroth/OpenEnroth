#pragma once

#include <string>
#include <functional> // For std::hash, std::equal_to, std::less.
#include <string_view>
#include <utility>

// TODO(captainurist): This is not needed in C++26 as we'll have transparent operator[].
/**
 * This one is needed for fully transparent maps, otherwise in pre-C++26 you'll get compilation errors in the following
 * code:
 * ```
 * std::map<std::string, int, TransparentStringLess> map = makeMap();
 * std::string_view key = "something";
 * map[key]++; // This won't compile.
 * ```
 *
 * This code will work though if you use `TransparentString` as your map's key.
 */
class TransparentString : public std::string {
 public:
    using std::string::string; // NOLINT: linter chokes here.
    TransparentString(std::string_view view) : std::string(view) {} // NOLINT: intentionally implicit.
    TransparentString(const std::string &s) : std::string(s) {} // NOLINT: intentionally implicit.
    TransparentString(std::string &&s) : std::string(std::move(s)) {} // NOLINT: intentionally implicit.
    TransparentString(const TransparentString &) = default;
    TransparentString(TransparentString &&) = default;
};

struct TransparentStringHash : std::hash<std::string_view> {
    using is_transparent = void;
};

struct TransparentStringEquals : std::equal_to<std::string_view> {
    using is_transparent = void;
};

struct TransparentStringLess : std::less<std::string_view> {
    using is_transparent = void;
};
