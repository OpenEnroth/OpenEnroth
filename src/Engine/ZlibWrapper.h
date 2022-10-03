#pragma once

#include "Utility/Blob.h"

namespace zlib {
int Compress(void *dest, unsigned int *destLen, void *source, unsigned int sourceLen);
int Uncompress(void *dest, unsigned int *destLen, const void *source, unsigned int sourceLen);

Blob Compress(const Blob &source);
Blob Uncompress(const Blob &source);
};  // namespace zlib
