#include "Encoding.h"

#include <cassert>
#include <span>
#include <string>

#include <ztd/text.hpp>

template<typename Callback>
static auto dispatchEncoding(TextEncoding encoding, Callback callback) {
    switch (encoding) {
    default:
        assert(false);
        [[fallthrough]];
    case ENCODING_ASCII:        return callback(ztd::text::ascii);
    case ENCODING_UTF8:         return callback(ztd::text::compat_utf8);
    case ENCODING_UTF16_BE:     return callback(ztd::text::basic_utf16_be<char>{});
    case ENCODING_UTF16_LE:     return callback(ztd::text::basic_utf16_le<char>{});
    case ENCODING_UTF32_BE:     return callback(ztd::text::basic_utf32_be<char>{});
    case ENCODING_UTF32_LE:     return callback(ztd::text::basic_utf32_le<char>{});
    case ENCODING_ISO_8859_1:   return callback(ztd::text::iso_8859_1);
    case ENCODING_ISO_8859_2:   return callback(ztd::text::iso_8859_2);
    case ENCODING_ISO_8859_3:   return callback(ztd::text::iso_8859_3);
    case ENCODING_ISO_8859_4:   return callback(ztd::text::iso_8859_4);
    case ENCODING_ISO_8859_5:   return callback(ztd::text::iso_8859_5);
    case ENCODING_ISO_8859_6:   return callback(ztd::text::iso_8859_6);
    case ENCODING_ISO_8859_7:   return callback(ztd::text::iso_8859_7);
    case ENCODING_ISO_8859_8:   return callback(ztd::text::iso_8859_8);
    case ENCODING_ISO_8859_10:  return callback(ztd::text::iso_8859_10);
    case ENCODING_ISO_8859_13:  return callback(ztd::text::iso_8859_13);
    case ENCODING_ISO_8859_15:  return callback(ztd::text::iso_8859_15);
    case ENCODING_ISO_8859_16:  return callback(ztd::text::iso_8859_16);
    case ENCODING_WINDOWS_1251: return callback(ztd::text::windows_1251);
    case ENCODING_WINDOWS_1252: return callback(ztd::text::windows_1252);
    case ENCODING_WINDOWS_1253: return callback(ztd::text::windows_1253);
    case ENCODING_WINDOWS_1255: return callback(ztd::text::windows_1255);
    case ENCODING_WINDOWS_1256: return callback(ztd::text::windows_1256);
    case ENCODING_WINDOWS_1257: return callback(ztd::text::windows_1257);
    case ENCODING_WINDOWS_1258: return callback(ztd::text::windows_1258);
    case ENCODING_IBM865:       return callback(ztd::text::windows_865_dos_nordic);
    case ENCODING_IBM866:       return callback(ztd::text::ibm_866_cyrillic);
    case ENCODING_BIG5:         return callback(ztd::text::big5_hkscs);
    case ENCODING_EUC_KR:       return callback(ztd::text::euc_kr_uhc);
    case ENCODING_GB18030:      return callback(ztd::text::gb18030);
    case ENCODING_SHIFT_JIS:    return callback(ztd::text::shift_jis);
    case ENCODING_KOI8_R:       return callback(ztd::text::koi8_r);
    }
}

std::string txt::wideToUtf8(std::wstring_view wstr) {
    std::span<const wchar_t> input(wstr.data(), wstr.size());
    if constexpr (sizeof(wchar_t) == 2) {
        return ztd::text::transcode(input, ztd::text::wide_utf16, ztd::text::compat_utf8, ztd::text::replacement_handler);
    } else {
        return ztd::text::transcode(input, ztd::text::wide_utf32, ztd::text::compat_utf8, ztd::text::replacement_handler);
    }
}

std::wstring txt::utf8ToWide(std::string_view str) {
    std::span<const char> input(str.data(), str.size());
    if constexpr (sizeof(wchar_t) == 2) {
        return ztd::text::transcode(input, ztd::text::compat_utf8, ztd::text::wide_utf16, ztd::text::replacement_handler);
    } else {
        return ztd::text::transcode(input, ztd::text::compat_utf8, ztd::text::wide_utf32, ztd::text::replacement_handler);
    }
}

std::string txt::encodedToUtf8(std::string_view str, TextEncoding encoding) {
    std::span<const char> input(str.data(), str.size());
    return dispatchEncoding(encoding == ENCODING_BYTES ? ENCODING_UTF8 : encoding, [&](auto enc) {
        return ztd::text::transcode(input, enc, ztd::text::compat_utf8, ztd::text::replacement_handler);
    });
}

std::string txt::utf8ToEncoded(std::string_view str, TextEncoding encoding) {
    std::span<const char> input(str.data(), str.size());
    return dispatchEncoding(encoding == ENCODING_BYTES ? ENCODING_UTF8 : encoding, [&](auto enc) {
        return ztd::text::transcode(input, ztd::text::compat_utf8, enc, ztd::text::replacement_handler);
    });
}
