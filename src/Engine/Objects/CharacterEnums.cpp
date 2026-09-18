#include "CharacterEnums.h"

#include "Library/Serialization/EnumSerialization.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(Mastery, CASE_INSENSITIVE, {
    {MASTERY_NOVICE, "N"},
    {MASTERY_EXPERT, "E"},
    {MASTERY_MASTER, "M"},
    {MASTERY_GRANDMASTER, "G"}
})
