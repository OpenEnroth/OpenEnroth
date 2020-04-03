#include "Engine/ZlibWrapper.h"
#include <string.h>

namespace zlib {
#include <zlib.h>

int Uncompress(void *dest, unsigned int *destLen, const void *source, unsigned int sourceLen) {
    return uncompress((Bytef *)dest, (uLongf *)destLen, (Bytef *)source, sourceLen);
}

int Compress(void *dest, unsigned int *destLen, void *source, unsigned int sourceLen) {
    return compress((Bytef *)dest, (uLongf *)destLen, (Bytef *)source, sourceLen);
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
        dest = (Bytef*)malloc(destLen);
        res = compress(dest, &destLen, (Bytef*)source->GetData(), source->GetSize());
    }

    PMemBuffer result = nullptr;
    if (res == Z_OK) {
        result = AllocMemBuffer(destLen);
        memcpy((void*)result->GetData(), dest, destLen);
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
        dest = (Bytef*)malloc(destLen);
        res = uncompress(dest, &destLen, (Bytef*)source->GetData(), source->GetSize());
    }

    PMemBuffer result = nullptr;
    if (res == Z_OK) {
        result = AllocMemBuffer(destLen);
        memcpy((void*)result->GetData(), dest, destLen);
    }
    free(dest);
    return result;
}

};  // namespace zlib
