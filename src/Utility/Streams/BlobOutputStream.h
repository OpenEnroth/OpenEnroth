#pragma once

#include <string>

#include "Utility/Memory/Blob.h"

#include "OutputStream.h"

/**
 * Output stream that writes into a `Blob`.
 *
 * Data is accumulated in an internal buffer and transferred to the target `Blob` when `close()` is called.
 * Calling `flush()` also transfers data but makes a copy instead of moving.
 */
class BlobOutputStream : public OutputStream {
 public:
    BlobOutputStream() = default;
    explicit BlobOutputStream(Blob *target, std::string_view displayPath = {});
    virtual ~BlobOutputStream();

    void open(Blob *target, std::string_view displayPath = {});

    virtual void write(const void *data, size_t size) override;
    using OutputStream::write;
    virtual void flush() override;
    virtual void close() override;
    [[nodiscard]] virtual std::string displayPath() const override;

 private:
    void closeInternal();

 private:
    Blob *_target = nullptr;
    std::string _buffer;
    std::string _displayPath;
};
