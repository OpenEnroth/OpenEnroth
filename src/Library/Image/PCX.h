#pragma once

#include <cstdint>
#include <memory>

#include "Library/Image/Image.h"
#include "Utility/Memory/Blob.h"

namespace PCX {
RgbaImage Decode(const Blob &data);
Blob Encode(RgbaImageView image);
}  // namespace PCX
