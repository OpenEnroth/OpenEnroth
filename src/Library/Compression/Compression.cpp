#include "Compression.h"

#include <zlib.h>

#include <cassert>
#include <cstring>
#include <algorithm>
#include <memory>
#include <utility>

#include "Utility/Exception.h"
#include "Utility/Memory/FreeDeleter.h"

namespace zlib {

Blob compress(const Blob &source) {
    size_t allocatedSize = source.size();
    uLongf destLen;
    std::unique_ptr<void, FreeDeleter> dest;
    int res = Z_BUF_ERROR;
    while (res == Z_BUF_ERROR) {
        if (dest) {
            dest.reset();
            allocatedSize *= 2;
        }
        dest.reset(malloc(allocatedSize));
        destLen = allocatedSize;
        res = ::compress(static_cast<Bytef *>(dest.get()), &destLen, static_cast<const Bytef *>(source.data()), source.size());
    }
    assert(res == Z_OK);

    if (destLen < allocatedSize)
        dest.reset(realloc(dest.release(), destLen));
    return Blob::fromMalloc(std::move(dest), destLen);
}

Blob uncompress(const Blob &source, size_t sizeHint) {
    size_t allocatedSize = sizeHint ? sizeHint : source.size() * 4;
    uLongf destLen;
    std::unique_ptr<void, FreeDeleter> dest;
    int res = Z_BUF_ERROR;
    while (res == Z_BUF_ERROR) {
        if (dest) {
            dest.reset();
            allocatedSize *= 2;
        }
        dest.reset(malloc(allocatedSize));
        destLen = allocatedSize;
        res = ::uncompress(static_cast<Bytef *>(dest.get()), &destLen, static_cast<const Bytef *>(source.data()), source.size());
    }

    if (res != Z_OK)
        throw Exception("Decompression error for '{}': {}", source.displayPath(), zError(res));

    if (destLen < allocatedSize)
        dest.reset(realloc(dest.release(), destLen));
    return Blob::fromMalloc(std::move(dest), destLen);
}

Blob uncompressBestEffort(const Blob &source, size_t sizeHint) {
    // Use raw deflate mode (inflateInit2 with negative window bits) so we can skip the
    // 2-byte zlib header and feed all remaining bytes directly as deflate input, without
    // expecting or verifying a trailing Adler-32 checksum. This recovers data from entries
    // where the zlib framing is corrupt or absent but the deflate content itself is valid.
    if (source.size() < 2)
        return Blob();

    z_stream stream = {};
    stream.next_in = const_cast<Bytef *>(static_cast<const Bytef *>(source.data())) + 2; // skip zlib header
    stream.avail_in = static_cast<uInt>(source.size() - 2);

    if (inflateInit2(&stream, -MAX_WBITS) != Z_OK)
        return Blob();

    size_t allocatedSize = sizeHint ? sizeHint : source.size() * 4;
    std::unique_ptr<void, FreeDeleter> dest(malloc(allocatedSize));
    stream.next_out = static_cast<Bytef *>(dest.get());
    stream.avail_out = static_cast<uInt>(allocatedSize);

    int res;
    do {
        if (stream.avail_out == 0) {
            uLong prevTotal = stream.total_out;
            allocatedSize *= 2;
            dest.reset(realloc(dest.release(), allocatedSize));
            stream.next_out = static_cast<Bytef *>(dest.get()) + prevTotal;
            stream.avail_out = static_cast<uInt>(allocatedSize - prevTotal);
        }
        res = inflate(&stream, Z_NO_FLUSH);
    } while (res == Z_OK);

    uLong outputSize = stream.total_out;
    inflateEnd(&stream);

    if (outputSize == 0)
        return Blob();

    if (outputSize < allocatedSize)
        dest.reset(realloc(dest.release(), outputSize));
    return Blob::fromMalloc(std::move(dest), outputSize).withDisplayPath(source.displayPath());
}

};  // namespace zlib
