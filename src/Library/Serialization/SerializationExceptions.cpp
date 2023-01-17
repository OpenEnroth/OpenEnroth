#include "SerializationExceptions.h"

#include <stdexcept>
#include <string>

#include "Utility/Format.h"

void throwEnumSerializationError(int64_t value, std::string_view typeName, bool isFlags) {
    throw std::runtime_error(fmt::format("Cannot serialize provided {} value `{}` of type {} to string", isFlags ? "flags" : "enum", value, typeName));
}

void throwDeserializationError(std::string_view value, std::string_view typeName) {
    throw std::runtime_error(fmt::format("Cannot deserialize '{}' as type {}", value, typeName));
}

void throwNumberDeserializationError(std::string_view invalidValue, std::string_view typeName, std::errc error) {
    if (error == std::errc::invalid_argument)
        throw std::runtime_error(fmt::format("'{}' is not a number"_cf, invalidValue));

    if (error == std::errc::result_out_of_range)
        throw std::runtime_error(fmt::format("'{}' does not fit in the range of {}"_cf, invalidValue, typeName));

    throw std::system_error(std::make_error_code(error), std::string(invalidValue));
}
