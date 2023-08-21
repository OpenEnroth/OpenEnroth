#pragma once

#include <stddef.h>
#include <string_view>

[[noreturn]] void throwBinarySerializationNoMoreDataError(size_t bytesRead, size_t bytesExpected, std::string_view typeName);
