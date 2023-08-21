#include "SerializationExceptions.h"

#include "Utility/Exception.h"

void throwEnumSerializationError(int64_t value, std::string_view typeName, bool isFlags) {
    throw Exception("Cannot serialize provided {} value `{}` of type {} to string", isFlags ? "flags" : "enum", value, typeName);
}

void throwDeserializationError(std::string_view value, std::string_view typeName) {
    throw Exception("Cannot deserialize '{}' as type {}", value, typeName);
}

void throwNumberDeserializationError(std::string_view invalidValue, std::string_view typeName, std::errc error) {
    if (error == std::errc::invalid_argument)
        throw Exception("'{}' is not a number", invalidValue);

    if (error == std::errc::result_out_of_range)
        throw Exception("'{}' does not fit in the range of {}", invalidValue, typeName);

    Exception::throwFromErrc(error, invalidValue);
}
