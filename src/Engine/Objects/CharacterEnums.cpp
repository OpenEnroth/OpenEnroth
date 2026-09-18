#include "CharacterEnums.h"

#include "Library/Serialization/EnumSerialization.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(Mastery, CASE_INSENSITIVE, {
    {MASTERY_NOVICE, "novice"},
    {MASTERY_EXPERT, "expert"},
    {MASTERY_MASTER, "master"},
    {MASTERY_GRANDMASTER, "grandmaster"}
})
