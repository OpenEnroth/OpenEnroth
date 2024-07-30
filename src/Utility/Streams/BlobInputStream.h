#pragma once

#include <string>

#include "Utility/Memory/Blob.h"
#include "Utility/Types.h"

#include "InputStream.h"

/**
 * Input stream that reads from a `Blob`.
 *
 * Besides the `InputStream` interface, this class also offers an interface for reading subblobs which share memory
 * with the source blob, and thus no memory copying occurs.
 */
class BlobInputStream : public InputStream {
 public:
    BlobInputStream() = default;
    explicit BlobInputStream(Blob &&blob);
    explicit BlobInputStream(const Blob &blob); // Shares the blob and stores the shared copy in this stream object.

    void open(Blob &&blob);
    void open(const Blob &blob);

    virtual size_t read(void *data, size_t size) override;
    virtual size_t skip(size_t size) override;
    virtual void close() override;
    [[nodiscard]] virtual std::string displayPath() const override;

    void seek(ssize_t pos);
    [[nodiscard]] ssize_t position() const;
    [[nodiscard]] ssize_t size() const;

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
    [[nodiscard]] size_t remaining() const;

 private:
    Blob _blob;
    const char *_pos = nullptr;
    const char *_end = nullptr;
};
