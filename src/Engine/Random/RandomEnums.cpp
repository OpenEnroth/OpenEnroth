#include "RandomEnums.h"

#include "Library/Serialization/EnumSerialization.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(RandomEngineType, CASE_INSENSITIVE, {
    {RANDOM_ENGINE_MERSENNE_TWISTER, "mersenne_twister"},
    {RANDOM_ENGINE_SEQUENTIAL, "sequential"}
})
