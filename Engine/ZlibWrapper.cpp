#include "Engine/ZlibWrapper.h"
#include <string.h>

namespace zlib {
#include <zlib.h>

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

PMemBuffer Compress(PMemBuffer source) {
    uLongf destLen = source->GetSize();
    Bytef *dest = nullptr;
    int res = Z_BUF_ERROR;
    while (res == Z_BUF_ERROR) {
        if (dest != nullptr) {
            free(dest);
            dest = nullptr;
            destLen *= 2;
        }
        dest = static_cast<Bytef *>(malloc(destLen));
        res = compress(dest, &destLen, static_cast<const Bytef *>(source->GetData()), source->GetSize());
    }

    PMemBuffer result = nullptr;
    if (res == Z_OK) {
        result = AllocMemBuffer(destLen);
        memcpy(const_cast<void *>(result->GetData()), dest, destLen);
    }
    free(dest);
    return result;
}

PMemBuffer Uncompress(PMemBuffer source) {
    uLongf destLen = source->GetSize() * 4;
    Bytef *dest = nullptr;
    int res = Z_BUF_ERROR;
    while (res == Z_BUF_ERROR) {
        if (dest != nullptr) {
            free(dest);
            dest = nullptr;
            destLen *= 2;
        }
        dest = static_cast<Bytef *>(malloc(destLen));
        res = uncompress(dest, &destLen, static_cast<const Bytef *>(source->GetData()), source->GetSize());
    }

    PMemBuffer result = nullptr;
    if (res == Z_OK) {
        result = AllocMemBuffer(destLen);
        memcpy(const_cast<void *>(result->GetData()), dest, destLen);
    }
    free(dest);
    return result;
}

};  // namespace zlib
