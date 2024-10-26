#pragma once

#include "Library/Image/Image.h"
#include "Utility/Memory/Blob.h"

namespace png {
RgbaImage decode(const Blob &data);
Blob encode(RgbaImageView image);
Blob encode(GrayscaleImageView image);
} // namespace png
