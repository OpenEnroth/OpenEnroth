#pragma once

namespace zlib
{
	int Compress(void *dest, unsigned int *destLen, void *source, unsigned int sourceLen);
	int Uncompress(void *dest, unsigned int *destLen, const void *source, unsigned int sourceLen);
};