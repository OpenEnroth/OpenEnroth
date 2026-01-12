#include "Utf.h"

#include <span>
#include <string>

#include <ztd/text.hpp>

std::string utf::wideToUtf8(std::wstring_view wstr) {
    // Use span to ensure proper boundary handling with ztd::text.
    std::span<const wchar_t> input(wstr.data(), wstr.size());
    if constexpr (sizeof(wchar_t) == 2) {
        // Windows: wchar_t is 16-bit, use UTF-16
        return ztd::text::transcode(input, ztd::text::wide_utf16, ztd::text::compat_utf8,
                                    ztd::text::replacement_handler);
    } else {
        // Unix: wchar_t is 32-bit, use UTF-32
        return ztd::text::transcode(input, ztd::text::wide_utf32, ztd::text::compat_utf8,
                                    ztd::text::replacement_handler);

    }
}

std::wstring utf::utf8ToWide(std::string_view str) {
    // Use span to ensure proper boundary handling with ztd::text.
    std::span<const char> input(str.data(), str.size());
    if constexpr (sizeof(wchar_t) == 2) {
        // Windows: wchar_t is 16-bit, use UTF-16
        return ztd::text::transcode(input, ztd::text::compat_utf8, ztd::text::wide_utf16,
                                    ztd::text::replacement_handler);
    } else {
        // Unix: wchar_t is 32-bit, use UTF-32
        return ztd::text::transcode(input, ztd::text::compat_utf8, ztd::text::wide_utf32,
                                    ztd::text::replacement_handler);
    }
}
