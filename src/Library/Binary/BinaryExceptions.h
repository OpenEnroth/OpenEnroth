#pragma once

#include <string_view>

[[noreturn]] void throwBinarySerializationNoMoreDataError(size_t bytesRead, size_t bytesExpected, std::string_view typeName);
[[noreturn]] void throwBinarySerializationLeftoverDataError(size_t bytesRead, size_t bytesLeft, std::string_view typeName);
