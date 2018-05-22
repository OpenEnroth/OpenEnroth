#pragma once

#include "src/tools/MemBuffer.h"

namespace zlib {
int Compress(void *dest, unsigned int *destLen, void *source, unsigned int sourceLen);
int Uncompress(void *dest, unsigned int *destLen, const void *source, unsigned int sourceLen);

PMemBuffer Compress(PMemBuffer source);
PMemBuffer Uncompress(PMemBuffer source);
};  // namespace zlib
