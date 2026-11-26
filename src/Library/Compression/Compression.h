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

/**
 * Checks for a zlib header (RFC 1950) that zlib can decode at the start of `data`. The header has to:
 * - Use deflate, the only compression method zlib defines.
 * - Have a window size of at most 32K, the largest RFC 1950 allows.
 * - Not ask for a preset dictionary, which isn't stored in the stream and which we never have.
 * - Pass its header checksum.
 *
 * Only the two header bytes are checked, so random data passes about once in 2000 tries, and a caller should have
 * other evidence too.
 *
 * @param data                          Data to check.
 * @return                              Whether `data` starts with a zlib header that zlib can decode.
 */
bool detect(const Blob &data);
};  // namespace zlib
