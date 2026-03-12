#pragma once

#include "Utility/Memory/Blob.h"

namespace zlib {
/**
 * @param source                       Blob to compress.
 * @return                             Compressed blob.
 */
Blob compress(const Blob &source);

/**
 * @param source                       Blob to decompress.
 * @param sizeHint                     Expected decompressed size. If zero, a default heuristic is used.
 * @return                             Decompressed blob.
 * @throws Exception                   On decompression error.
 */
Blob uncompress(const Blob &source, size_t sizeHint = 0);
};  // namespace zlib
