#include "JsonExceptions.h"

#include "Utility/String/Format.h"

#include "Json.h"

void throwJsonDeserializationError(const Json &json, const char *typeName) {
    throw Json::type_error::create(334, fmt::format("Cannot deserialize json value '{}' as type {}", to_string(json), typeName), &json);
}
