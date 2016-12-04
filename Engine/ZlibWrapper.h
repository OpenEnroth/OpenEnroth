#pragma once

namespace zlib
{
  int MemZip(void *dest, unsigned int *destLen, void *source, unsigned int sourceLen);
  int MemUnzip(void *dest, unsigned int *destLen, const void *source, unsigned int sourceLen);
};