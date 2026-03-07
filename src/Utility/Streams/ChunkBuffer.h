#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "Utility/Memory/FreeDeleter.h"

#include "StreamBuffer.h"

/**
 * Internal helper for accumulating data in a sequence of geometrically-growing heap chunks.
 */
class ChunkBuffer {
 public:
    void reset() { _chunks.clear(); }

    [[nodiscard]] StreamBuffer<char> allocateChunk(size_t minSize = 0) {
        size_t size = std::max(minSize, _chunks.empty() ? 1024 : std::min<size_t>(_chunks.back().size * 2, 1024 * 1024));

        Chunk &chunk = _chunks.emplace_back();
        chunk.data.reset(static_cast<char *>(malloc(size)));
        chunk.size = size;

        return StreamBuffer<char>(chunk.data.get(), chunk.data.get(), chunk.data.get() + size);
    }

    [[nodiscard]] std::unique_ptr<char, FreeDeleter> popChunk() {
        auto result = std::move(_chunks.back().data);
        _chunks.pop_back();
        return result;
    }

    [[nodiscard]] size_t chunkCount() const { return _chunks.size(); }

    void materialize(void *dst, size_t size) {
        StreamBuffer<char> buffer(static_cast<char *>(dst), static_cast<char *>(dst), static_cast<char *>(dst) + size);
        for (const Chunk &chunk : _chunks)
            buffer.write(chunk.data.get(), std::min(chunk.size, buffer.remaining()));
    }

private:
    struct Chunk {
        std::unique_ptr<char, FreeDeleter> data;
        size_t size = 0;
    };

    std::vector<Chunk> _chunks;
};
