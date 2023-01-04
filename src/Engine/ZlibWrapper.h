#pragma once

#include "Utility/Memory/Blob.h"

namespace zlib {
// TODO: Use size_t in API here.
int Compress(void *dest, unsigned int *destLen, void *source, unsigned int sourceLen);
int Uncompress(void *dest, unsigned int *destLen, const void *source, unsigned int sourceLen);

Blob Compress(const Blob &source);
Blob Uncompress(const Blob &source, size_t sizeHint = 0);
};  // namespace zlib
