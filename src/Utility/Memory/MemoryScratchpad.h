#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <span>
#include <utility>
#include <vector>

#include "Blob.h"
#include "FreeDeleter.h"

/**
 * A scratchpad for accumulating data of unknown total size in a sequence of geometrically growing heap chunks,
 * to be materialized once at the end.
 *
 * `MemoryScratchpad` intentionally doesn't track how much of the handed-out memory was actually used. The caller
 * is expected to keep count, and pass the total size into `close` / `copy`. This keeps the write cursor in one
 * place (e.g. in the stream buffer for `BlobOutputStream`) instead of duplicating it here.
 */
class MemoryScratchpad {
 public:
    explicit MemoryScratchpad(size_t maxChunkSize = 1024 * 1024) : _maxChunkSize(maxChunkSize) {
        assert(maxChunkSize > 0);
    }

    /**
     * Allocates the next chunk. Note that the chunks only grow, with each next chunk twice the size of the previous
     * one, up to the max chunk size passed to the constructor.
     *
     * @param minSize                   Minimal size of the chunk to allocate.
     * @return                          Newly allocated chunk of at least `minSize` bytes. The chunk is owned by this
     *                                  `MemoryScratchpad`, and the returned span is invalidated by the calls to
     *                                  `close` and `reset`.
     */
    [[nodiscard]] std::span<char> next(size_t minSize = 0) {
        size_t size = std::max(minSize, std::min(_chunks.empty() ? 1024 : _chunks.back().size * 2, _maxChunkSize));

        Chunk &chunk = _chunks.emplace_back();
        chunk.data.reset(static_cast<char *>(malloc(size)));
        chunk.size = size;
        _capacity += size;

        return {chunk.data.get(), size};
    }

    /**
     * Copies out the first `size` bytes of the accumulated data without consuming the chunks. Unlike `close`, this
     * function can be called multiple times, and thus this is what a `flush` implementation would use.
     *
     * @param dst                       Destination to copy the data into.
     * @param size                      Number of bytes to copy. Must not exceed the total number of bytes written
     *                                  into the chunks.
     */
    void materialize(void *dst, size_t size) const {
        assert(dst || size == 0);
        assert(size <= _capacity);

        char *pos = static_cast<char *>(dst);
        for (const Chunk &chunk : _chunks) {
            if (size == 0)
                break;

            size_t chunkSize = std::min(chunk.size, size);
            memcpy(pos, chunk.data.get(), chunkSize);
            pos += chunkSize;
            size -= chunkSize;
        }
    }

    /**
     * Closes this scratchpad, materializing everything that was written into it as a `Blob`, and resetting the
     * scratchpad. When all of the data fits in a single chunk, chunk ownership is transferred into the resulting
     * `Blob` without copying.
     *
     * @param size                      Total number of bytes written into the chunks, in allocation order.
     * @return                          `Blob` containing the first `size` bytes of the accumulated data.
     */
    [[nodiscard]] Blob finish(size_t size) {
        assert(size <= _capacity);

        Blob result;
        if (size == 0) {
            // Nothing to do.
        } else if (_chunks.size() == 1) {
            result = Blob::fromMalloc(std::move(_chunks.front().data), size);
        } else {
            std::unique_ptr<char, FreeDeleter> data(static_cast<char *>(malloc(size)));
            materialize(data.get(), size);
            result = Blob::fromMalloc(std::move(data), size);
        }

        reset();
        return result;
    }

    /**
     * Resets this scratchpad, freeing all allocated chunks.
     */
    void reset() {
        _chunks.clear();
        _capacity = 0;
    }

 private:
    struct Chunk {
        std::unique_ptr<char, FreeDeleter> data;
        size_t size = 0;
    };

 private:
    size_t _maxChunkSize = 0;
    std::vector<Chunk> _chunks;
    size_t _capacity = 0;
};
