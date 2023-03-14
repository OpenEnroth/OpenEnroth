#include "Compression.h"

#include <zlib.h>

#include <cstring>
#include <algorithm>
#include <memory>

#include "Utility/Memory/FreeDeleter.h"

namespace zlib {

int Uncompress(void *dest, unsigned int *destLen, const void *source, unsigned int sourceLen) {
    uLongf localDestLen = *destLen;
    int result = uncompress(static_cast<Bytef *>(dest), &localDestLen, static_cast<const Bytef *>(source), sourceLen);
    *destLen = localDestLen;
    return result;
}

int Compress(void *dest, unsigned int *destLen, void *source, unsigned int sourceLen) {
    uLongf localDestLen = *destLen;
    int result = compress(static_cast<Bytef *>(dest), &localDestLen, static_cast<const Bytef *>(source), sourceLen);
    *destLen = localDestLen;
    return result;
}

Blob Compress(const Blob &source) {
    uLongf destLen = source.size();
    std::unique_ptr<void, FreeDeleter> dest;
    int res = Z_BUF_ERROR;
    while (res == Z_BUF_ERROR) {
        if (dest) {
            dest.reset();
            destLen *= 2;
        }
        dest.reset(malloc(destLen));
        res = compress(static_cast<Bytef *>(dest.get()), &destLen, static_cast<const Bytef *>(source.data()), source.size());
    }

    return res == Z_OK ? Blob::copy(dest.get(), destLen) : Blob();
}

Blob Uncompress(const Blob &source, size_t sizeHint) {
    uLongf destLen = sizeHint > source.size() ? sizeHint : source.size() * 4;
    std::unique_ptr<void, FreeDeleter> dest;
    int res = Z_BUF_ERROR;
    while (res == Z_BUF_ERROR) {
        if (dest) {
            dest.reset();
            destLen *= 2;
        }
        dest.reset(malloc(destLen));
        res = uncompress(static_cast<Bytef *>(dest.get()), &destLen, static_cast<const Bytef *>(source.data()), source.size());
    }

    return res == Z_OK ? Blob::copy(dest.get(), destLen) : Blob();
}

};  // namespace zlib
