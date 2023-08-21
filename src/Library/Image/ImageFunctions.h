#pragma once

#include "Image.h"
#include "Palette.h"

struct Palette;

RgbaImage makeRgbaImage(GrayscaleImageView indexedImage, const Palette &palette);

RgbaImage flipVertically(RgbaImageView image);
