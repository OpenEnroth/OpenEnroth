#pragma once

#include "Library/Image/Image.h"
#include "Library/Image/Palette.h"

struct Texture_MM7 {
    void Release();

    std::string name;
    GrayscaleImage indexed;
    Palette palette;
    bool zeroIsTransparent = false;
};
