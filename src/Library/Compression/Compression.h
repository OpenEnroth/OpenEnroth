#pragma once

#include "Utility/Memory/Blob.h"

namespace zlib {
Blob compress(const Blob &source);
Blob uncompress(const Blob &source, size_t sizeHint = 0);
};  // namespace zlib
