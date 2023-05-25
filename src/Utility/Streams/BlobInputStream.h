#pragma once

#include "Utility/Memory/Blob.h"

#include "InputStream.h"

class BlobInputStream : public InputStream {
 public:
    BlobInputStream() = default;
    explicit BlobInputStream(const Blob &blob); // TODO(captainurist): Maybe `const Blob *` here?

    virtual size_t read(void *data, size_t size) override;
    virtual size_t skip(size_t size) override;
    virtual void close() override;

    /**
     * @return                          Remaining stream data, as a blob that's shared with the blob that this
     *                                  stream is reading from.
     */
    [[nodiscard]] Blob tail() const;

    /**
     * Same as `read`, but returns the data as a blob that's shared with the blob that this stream is reading from.
     *
     * This basically presents a way to conveniently cut a blob into subblobs using a streaming interface.
     *
     * @param size                      Number of bytes to read.
     * @return                          Subblob of the blob that this stream is reading from. Actual size might be
     *                                  less than `size` if end of stream is encountered.
     */
    [[nodiscard]] Blob readBlob(size_t size);

    /**
     * Same as `readOrFail`, but returns the data as a blob that's shared with the blob that this stream is
     * reading from.
     *
     * @param size                      Number of bytes to read.
     * @return                          Subblob of the blob that this stream is reading from.
     * @throw Exception                 If there is not enough data in the stream.
     */
    [[nodiscard]] Blob readBlobOrFail(size_t size);

 private:
    [[nodiscard]] size_t offset() const;
    [[nodiscard]] size_t remaining() const;

 private:
    const Blob *_blob = nullptr;
    const char *_pos = nullptr;
    const char *_end = nullptr;
};
