#pragma once

#include <cstddef>
#include <string_view>

/**
 * Wrapper for ASCII string literals that checks its contents at compile time.
 *
 * The constructor is `consteval` and only takes character arrays, so runtime strings and `const char *` pointers
 * don't compile. Neither do literals with non-ASCII or NUL characters in them - ASCII is the one subset that means
 * the same bytes in every source & execution charset.
 */
class AsciiLiteral {
 public:
    template<size_t N>
    consteval AsciiLiteral(const char (&string)[N]) : _string(string, N - 1) { // NOLINT: intentionally implicit.
        for (char c : _string)
            if (c == '\0' || static_cast<unsigned char>(c) >= 0x80)
                nonAsciiCharactersInLiteral();
    }

    constexpr operator std::string_view() const {
        return _string;
    }

 private:
    static void nonAsciiCharactersInLiteral(); // Deliberately not defined, referencing it fails compilation.

 private:
    std::string_view _string;
};
