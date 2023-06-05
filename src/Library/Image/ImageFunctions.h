#pragma once

#include "Image.h"
#include "Palette.h"

RgbaImage makeRgbaImage(GrayscaleImageView indexedImage, const Palette &palette);

RgbaImage flipVertically(RgbaImageView image);
