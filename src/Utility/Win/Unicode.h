#pragma once

#include <string>
#include <string_view>

namespace win {

// These are only available on Windows:

std::string toUtf8(std::wstring_view wstr);
std::wstring toUtf16(std::string_view str);

} // namespace win
