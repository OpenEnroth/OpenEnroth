#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Utility/Memory/Blob.h"
#include "Utility/Memory/FreeDeleter.h"

#include "OutputStream.h"

/**
 * Output stream that writes into a `Blob`.
 *
 * Data is accumulated in geometrically growing internal chunks (1KB up to 1MB) and transferred to the target `Blob`
 * when `close()` is called. Calling `flush()` also transfers data but makes a copy instead of moving.
 */
class BlobOutputStream : public OutputStream {
 public:
    BlobOutputStream() = default;

    /**
     * @param target                    Blob to write into. Must outlive this stream.
     * @param displayPath               Display path for error reporting.
     */
    explicit BlobOutputStream(Blob *target, std::string_view displayPath = {});
    ~BlobOutputStream();

    /**
     * Opens the stream for writing into the given blob.
     *
     * @param target                    Blob to write into. Must outlive this stream.
     * @param displayPath               Display path for error reporting.
     */
    void open(Blob *target, std::string_view displayPath = {});

 protected:
    virtual void _overflow(const void *data, size_t size, void **bufferStart, void **bufferEnd) override;
    virtual void _flush() override;
    virtual void _close() override;

 private:
    void closeInternal();
    Blob materialize();

 private:
    struct Chunk {
        std::unique_ptr<char, FreeDeleter> data;
        size_t size = 0;
    };

    Blob *_target = nullptr;
    std::vector<Chunk> _chunks;
};
