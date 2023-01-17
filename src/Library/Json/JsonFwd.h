#pragma once

#include <nlohmann/json_fwd.hpp>

using Json = nlohmann::json;

/**
 * @param TYPE                          Type to forward-declare json serialization functions for.
 */
#define MM_DECLARE_JSON_SERIALIZATION_FUNCTIONS(TYPE)                                                                   \
void to_json(Json &json, const TYPE &value);                                                                            \
void from_json(const Json &j, TYPE &value);

