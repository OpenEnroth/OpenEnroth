#pragma once

#include <cstdint>
#include <string_view>
#include <system_error>

#include "Utility/Flags.h"

[[noreturn]] void throwEnumSerializationError(int64_t value, std::string_view typeName, bool isFlags);
[[noreturn]] void throwDeserializationError(std::string_view value, std::string_view typeName);
[[noreturn]] void throwNumberDeserializationError(std::string_view invalidValue, std::string_view typeName, std::errc error);

template<class T>
void throwEnumSerializationError(T value, std::string_view typeName) {
    if constexpr (std::is_enum_v<T> || std::is_integral_v<T>) {
        throwEnumSerializationError(static_cast<int64_t>(value), typeName, false);
    } else {
        // Assume it's flags.
        throwEnumSerializationError(static_cast<int64_t>(std::to_underlying(value)), typeName, true);
    }
}

