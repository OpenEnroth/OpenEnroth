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

/**
 * Best-effort decompression that recovers whatever bytes the deflate stream produces, even if the
 * zlib checksum is invalid or the stream is otherwise corrupt. Useful for known-corrupt game data
 * where partial output is better than nothing.
 *
 * @param source                       Blob to decompress.
 * @param sizeHint                     Expected decompressed size. If zero, a default heuristic is used.
 * @return                             Decompressed blob, possibly smaller than expected. Empty if no
 *                                     bytes could be recovered at all.
 */
Blob uncompressBestEffort(const Blob &source, size_t sizeHint = 0);
};  // namespace zlib
