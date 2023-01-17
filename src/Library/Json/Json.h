#pragma once

#include <string>

#define JSON_DISABLE_ENUM_SERIALIZATION 1
#include <nlohmann/json.hpp>

#include "Library/Serialization/Serialization.h"

#include "Utility/Preprocessor.h"

#include "JsonFwd.h"
#include "JsonExceptions.h"

/**
 * Defines enum json serialization functions that just forward into lexical serialization declared via
 * `Library/Serialization`.
 *
 * Note that `NLOHMANN_JSON_SERIALIZE_ENUM` offers no customization of how errors are reported, and this is why we're
 * not using it.
 *
 * @param TYPE                          Type to define json serialization functions for.
 */
#define MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(TYPE)                                                            \
    void to_json(Json &json, const TYPE &value) {                                                                       \
        nlohmann::to_json(json, toString(value));                                                                       \
    }                                                                                                                   \
                                                                                                                        \
    void from_json(const Json &json, TYPE &value) {                                                                     \
        const std::string *jsonString = json.get<const std::string *>();                                                \
        if (!jsonString)                                                                                                \
            throwJsonDeserializationError(json, #TYPE);                                                                 \
        value = fromString<TYPE>(*jsonString);                                                                          \
    }


/**
 * Unfortunately `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT` is also very weak when it comes to customization,
 * so we have to roll out our own version.
 *
 * @param TYPE                          Struct to define functions for.
 * @param MEMBER_NAMES                  Member-name pairs. Repeats are not supported for now.
 */
#define MM_DEFINE_JSON_STRUCT_SERIALIZATION_FUNCTIONS(TYPE, MEMBER_NAMES)                                               \
    void to_json(Json &json, const TYPE &value) {                                                                       \
        NLOHMANN_JSON_PASTE(MM_DEFINE_JSON_STRUCT_MEMBER_SERIALIZATION_I, MM_PP_REMOVE_PARENS(MEMBER_NAMES))            \
    }                                                                                                                   \
                                                                                                                        \
    void from_json(const Json &json, TYPE &value) {                                                                     \
        if (!json.is_object())                                                                                          \
            throwJsonDeserializationError(json, #TYPE);                                                                 \
        NLOHMANN_JSON_PASTE(MM_DEFINE_JSON_STRUCT_MEMBER_DESERIALIZATION_I, MM_PP_REMOVE_PARENS(MEMBER_NAMES))          \
    }


#define MM_DEFINE_JSON_STRUCT_MEMBER_SERIALIZATION_I(ARG) MM_DEFINE_JSON_STRUCT_MEMBER_SERIALIZATION_II ARG
#define MM_DEFINE_JSON_STRUCT_MEMBER_SERIALIZATION_II(MEMBER, NAME)                                                     \
    nlohmann::to_json(json[NAME], value.MEMBER);

#define MM_DEFINE_JSON_STRUCT_MEMBER_DESERIALIZATION_I(ARG) MM_DEFINE_JSON_STRUCT_MEMBER_DESERIALIZATION_II ARG
#define MM_DEFINE_JSON_STRUCT_MEMBER_DESERIALIZATION_II(MEMBER, NAME)                                                   \
    if (json.contains(NAME))                                                                                            \
        nlohmann::from_json(json[NAME], value.MEMBER);

