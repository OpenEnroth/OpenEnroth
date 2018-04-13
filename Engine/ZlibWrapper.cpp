#include "Engine/ZlibWrapper.h"

namespace zlib {
#include "lib/zlib/zlib.h"

int Uncompress(void *dest, unsigned int *destLen, const void *source,
               unsigned int sourceLen) {
    return uncompress((Bytef *)dest, (uLongf *)destLen, (Bytef *)source,
                      sourceLen);
}

int Compress(void *dest, unsigned int *destLen, void *source,
             unsigned int sourceLen) {
    return compress((Bytef *)dest, (uLongf *)destLen, (Bytef *)source,
                    sourceLen);
}
};  // namespace zlib
