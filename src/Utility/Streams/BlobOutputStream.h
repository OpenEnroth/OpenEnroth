#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

#include "Utility/Memory/Blob.h"

#include "ChunkBuffer.h"
#include "OutputStream.h"

/**
 * Output stream that writes into a `Blob`.
 *
 * Data is accumulated in geometrically growing internal chunks (1KB up to 1MB) and transferred to the target `Blob`
 * when `close()` is called. Calling `flush()` also transfers data but always makes a copy. `close()` can avoid the
 * copy when all data fits in a single chunk.
 */
class BlobOutputStream : public OutputStream {
    using base_type = OutputStream;

 public:
    BlobOutputStream() = default;

    /**
     * @param target                    Blob to write into. Must outlive this stream.
     * @param displayPath               Display path for error reporting.
     */
    explicit BlobOutputStream(Blob *target, std::string_view displayPath = {});

    virtual ~BlobOutputStream();

    /**
     * Opens the stream for writing into the given blob.
     *
     * @param target                    Blob to write into. Must outlive this stream.
     * @param displayPath               Display path for error reporting.
     */
    void open(Blob *target, std::string_view displayPath = {});

 protected:
    virtual void _overflow(const void *data, size_t size, Buffer *buffer) override;
    virtual void _flush(Buffer *buffer) override;
    virtual void _close(Buffer *buffer) override;

 private:
    Blob materialize();
    void closeInternal();

 private:
    Blob *_target = nullptr;
    ChunkBuffer _chunks;
};
