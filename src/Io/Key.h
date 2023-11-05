#pragma once

#include <string>
#include <string_view>

#include "Library/Platform/Interface/PlatformEnums.h"

#include "Library/Serialization/SerializationFwd.h"

// TODO(captainurist): find a better place for this code

MM_DECLARE_SERIALIZATION_FUNCTIONS(PlatformKey)

std::string GetDisplayName(PlatformKey key);
bool TryParseDisplayName(std::string_view displayName, PlatformKey *outKey);
