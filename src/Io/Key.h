#pragma once

#include <string>
#include <string_view>

#include "Platform/PlatformEnums.h"
#include "Library/Serialization/SerializationFwd.h"

enum class PlatformKey : int;

// TODO(captainurist): find a better place for this code

MM_DECLARE_SERIALIZATION_FUNCTIONS(PlatformKey)

std::string GetDisplayName(PlatformKey key);
bool TryParseDisplayName(std::string_view displayName, PlatformKey *outKey);
