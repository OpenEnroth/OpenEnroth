#include "Color.h"

#include <string>

#include "Library/Serialization/SerializationExceptions.h"

static int parseHex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int parseByte(char hi, char lo) {
    int h = parseHex(hi);
    int l = parseHex(lo);
    if (h < 0 || l < 0) return -1;
    return (h << 4) | l;
}

[[nodiscard]] bool trySerialize(const Color &src, std::string *dst) {
    if (src.a != 255) {
        *dst = fmt::format("#{:02x}{:02x}{:02x}{:02x}", src.r, src.g, src.b, src.a);
    } else {
        *dst = fmt::format("#{:02x}{:02x}{:02x}", src.r, src.g, src.b);
    }
    return true;
}

[[nodiscard]] bool tryDeserialize(std::string_view src, Color *dst) {
    if (src.empty() || src[0] != '#')
        return false;

    if (src.size() == 7) {  // #RRGGBB
        int r = parseByte(src[1], src[2]);
        int g = parseByte(src[3], src[4]);
        int b = parseByte(src[5], src[6]);
        if (r < 0 || g < 0 || b < 0) return false;
        *dst = Color(r, g, b);
        return true;
    } else if (src.size() == 9) {  // #RRGGBBAA
        int r = parseByte(src[1], src[2]);
        int g = parseByte(src[3], src[4]);
        int b = parseByte(src[5], src[6]);
        int a = parseByte(src[7], src[8]);
        if (r < 0 || g < 0 || b < 0 || a < 0) return false;
        *dst = Color(r, g, b, a);
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
