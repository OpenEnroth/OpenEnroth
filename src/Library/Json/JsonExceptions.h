#pragma once

#include "JsonFwd.h"

[[noreturn]] void throwJsonDeserializationError(const Json &json, const char *typeName);
