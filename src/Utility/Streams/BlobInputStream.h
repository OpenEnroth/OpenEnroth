#pragma once

#include <string>

#include "Utility/Memory/Blob.h"

#include "InputStream.h"

/**
 * Input stream that reads from a `Blob`.
 *
 * Besides the `InputStream` interface, this class also offers an interface for reading subblobs which share memory
 * with the source blob, and thus no memory copying occurs.
 */
class BlobInputStream : public InputStream {
    using base_type = InputStream;

 public:
    BlobInputStream() = default;
    explicit BlobInputStream(Blob &&blob);
    explicit BlobInputStream(const Blob &blob); // Shares the blob and stores the shared copy in this stream object.

    void open(Blob &&blob);
    void open(const Blob &blob);

    /**
     * Same as `read`, but returns the data as a blob that's shared with the blob that this stream is reading from.
     *
     * This basically presents a way to conveniently cut a blob into subblobs using a streaming interface.
     *
     * @param size                      Number of bytes to read.
     * @return                          Subblob of the blob that this stream is reading from. Actual size might be
     *                                  less than `size` if end of stream is encountered.
     */
    [[nodiscard]] Blob readAsBlob(size_t size);

    /**
     * Same as `readOrFail`, but returns the data as a blob that's shared with the blob that this stream is
     * reading from.
     *
     * @param size                      Number of bytes to read.
     * @return                          Subblob of the blob that this stream is reading from.
     * @throws Exception                If there is not enough data in the stream.
     */
    [[nodiscard]] Blob readAsBlobOrFail(size_t size);

   /**
    * Same as `readAll`, but returns the data as a blob that's shared with the blob that this stream is reading from.
    *
    * @return                          Remaining stream data as a shared subblob. No copying occurs.
    */
    [[nodiscard]] Blob readAllAsBlob();

 private:
    Blob _blob;
};
