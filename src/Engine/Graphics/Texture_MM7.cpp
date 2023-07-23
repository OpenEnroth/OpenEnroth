#include "Texture_MM7.h"

#include <cstring>
#include <cstdlib>

#include "Engine/ErrorHandling.h"

void Texture_MM7::Release() {
    header.name[0] = 0;

    if (header.flags & 0x0400) {
        __debugbreak();
    }

    paletted_pixels = GrayscaleImage();

    memset(&header, 0, sizeof(header));
}

Texture_MM7::Texture_MM7() {
    memset(&header, 0, sizeof(header));
}
