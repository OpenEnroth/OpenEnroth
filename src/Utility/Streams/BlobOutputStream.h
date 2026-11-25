#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

#include "Utility/Memory/Blob.h"

#include "OutputStream.h"

/**
 * Output stream that writes into a `Blob`.
 *
 * Data is accumulated in a single geometrically growing buffer and transferred to the target `Blob` when `close()`
 * is called. Calling `flush()` also transfers data, without copying: the target blob shares the buffer's memory, so
 * flushing after every write stays O(1).
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
    virtual void _overflow(Buffer *buffer, const void *data, size_t size) override;
    virtual void _flush(Buffer *buffer) override;
    virtual void _close(Buffer *buffer, bool canThrow) override;

 private:
    struct Storage;

    Blob publish(Buffer *buffer);

 private:
    Blob *_target = nullptr;
    std::shared_ptr<Storage> _storage;
};
