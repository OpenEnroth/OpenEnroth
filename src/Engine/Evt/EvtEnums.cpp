#include "EvtEnums.h"

#include "Library/Serialization/EnumSerialization.h"

MM_DEFINE_ENUM_MAGIC_SERIALIZATION_FUNCTIONS(EvtOpcode)

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(EvtTargetCharacter, CASE_SENSITIVE, {
    {CHOOSE_ACTIVE, "Current"},
    {CHOOSE_ACTIVE, "current"},
    {CHOOSE_PARTY, "All"},
    {CHOOSE_PARTY, "all"},
    {CHOOSE_RANDOM, "Random"},
    {CHOOSE_RANDOM, "random"}
})
