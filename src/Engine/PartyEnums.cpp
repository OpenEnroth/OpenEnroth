#include "PartyEnums.h"

#include "Library/Serialization/EnumSerialization.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(PartyAlignment, CASE_INSENSITIVE, {
    {PartyAlignment_Good, "good"},
    {PartyAlignment_Neutral, "neutral"},
    {PartyAlignment_Evil, "evil"},
})
