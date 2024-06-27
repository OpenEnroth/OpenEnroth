#include "Color.h"

#include "Library/Serialization/StandardSerialization.h"
#include "Library/Serialization/SerializationExceptions.h"

[[nodiscard]] bool trySerialize(const Color &src, std::string *dst) {
    if (src.a != 255) {
        *dst = fmt::format("0x{:02x}{:02x}{:02x}{:02x}", src.r, src.g, src.b, src.a);
    } else {
        *dst = fmt::format("0x{:02x}{:02x}{:02x}", src.r, src.g, src.b);
    }
    return true;
}

[[nodiscard]] bool tryDeserialize(std::string_view src, Color *dst) {
    int r, g, b, a;
    char ch;
    if (5 == sscanf(std::string(src).c_str(), "0x%2x%2x%2x%2x%c", &r, &g, &b, &a, &ch)) {
        // Garbage at the end
        return false;
    } else if (4 == sscanf(std::string(src).c_str(), "0x%2x%2x%2x%2x", &r, &g, &b, &a)) {
        *dst = Color(r, g, b, a);
        return true;
    } else if (3 == sscanf(std::string(src).c_str(), "0x%2x%2x%2x", &r, &g, &b)) {
        *dst = Color(r, g, b);
        return true;
    }
    return false;
}

void serialize(const Color &src, std::string *dst) {
    (void) trySerialize(src, dst);
}

void deserialize(std::string_view src, Color *dst) {
    if (!tryDeserialize(src, dst))
        throwDeserializationError(src, "Color");
}

