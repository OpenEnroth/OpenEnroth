#include "JsonExceptions.h"

#include <map>
#include <string>

#include "Library/Json/JsonFwd.h"
#include "fmt/core.h"
#include "nlohmann/json.hpp"

void throwJsonDeserializationError(const Json &json, const char *typeName) {
    throw Json::type_error::create(334, fmt::format("Cannot deserialize json value '{}' as type {}", to_string(json), typeName), &json);
}
