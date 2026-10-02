#include "ResourceMask.h"

#include <string>

#include "Library/Json/Json.h"
#include "Library/Serialization/EnumSerialization.h"
#include "Library/Serialization/SerializationExceptions.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(MaskMode, CASE_INSENSITIVE, {
    {MASK_DEFAULT, "default"},
    {MASK_NONE, "none"},
    {MASK_ZERO, "zero"},
    {MASK_COLOR, "color"}
})

[[nodiscard]] bool trySerialize(const ResourceMask &src, std::string *dst) {
    return src.mode == MASK_COLOR ? trySerialize(src.color, dst) : trySerialize(src.mode, dst);
}

void serialize(const ResourceMask &src, std::string *dst) {
    (void) trySerialize(src, dst);
}

[[nodiscard]] bool tryDeserialize(std::string_view src, ResourceMask *dst) {
    *dst = ResourceMask();
    if (tryDeserialize(src, &dst->mode))
        return dst->mode != MASK_COLOR; // A bare "color" has no color to mask.

    if (!tryDeserialize(src, &dst->color))
        return false;

    dst->mode = MASK_COLOR;
    return true;
}

void deserialize(std::string_view src, ResourceMask *dst) {
    if (!tryDeserialize(src, dst))
        throwDeserializationError(src, "ResourceMask");
}

MM_DEFINE_JSON_LEXICAL_SERIALIZATION_FUNCTIONS(ResourceMask)
