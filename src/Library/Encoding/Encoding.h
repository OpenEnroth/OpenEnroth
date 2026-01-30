#pragma once

#include <string>
#include <string_view>

#include "EncodingEnums.h"

namespace txt {

/**
 * Convert a wide string to UTF-8.
 *
 * On Windows, the wide string is assumed to be UTF-16. On other platforms, it's assumed to be UTF-32.
 * Invalid sequences are replaced with the Unicode replacement character (U+FFFD).
 *
 * @param wstr                          Wide string to convert.
 * @return                              UTF-8 encoded string.
 */
std::string wideToUtf8(std::wstring_view wstr);

/**
 * Convert a UTF-8 string to a wide string.
 *
 * On Windows, the output is UTF-16. On other platforms, the output is UTF-32.
 * Invalid sequences are replaced with the Unicode replacement character (U+FFFD).
 *
 * @param str                           UTF-8 string to convert.
 * @return                              Wide string.
 */
std::wstring utf8ToWide(std::string_view str);

std::string encodedToUtf8(std::string_view str, TextEncoding encoding);

std::string utf8ToEncoded(std::string_view str, TextEncoding encoding);

} // namespace txt
