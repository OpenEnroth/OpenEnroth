#pragma once

#include "Utility/Memory/Blob.h"

namespace zlib {
Blob Compress(const Blob &source);
Blob Uncompress(const Blob &source, size_t sizeHint = 0);
};  // namespace zlib
