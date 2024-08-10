#include "FFmpegBlobInputStream.h"

#include <algorithm>
#include <utility>

extern "C" {
#include <libavcodec/avcodec.h> // NOLINT: not a C system header.
#include <libavformat/avio.h> // NOLINT: not a C system header.
#include <libavutil/mem.h> // NOLINT: not a C system header.
}

static int ffRead(void *opaque, uint8_t *buf, int size) {
    FFmpegBlobInputStream *stream = static_cast<FFmpegBlobInputStream *>(opaque);
    int result = stream->read(buf, size);
    return result == 0 ? AVERROR_EOF : result;
}

static int64_t ffSeek(void *opaque, int64_t offset, int whence) {
    // Note: we cast to BlobInputStream, not FFmpegBlobInputStream, so that we could call BlobInputStream::seek below.
    BlobInputStream *stream = static_cast<BlobInputStream *>(opaque);

    if (whence & AVSEEK_SIZE)
        return stream->size();

    whence &= ~AVSEEK_FORCE;
    switch (whence) {
    case SEEK_SET:
        stream->seek(offset);
        break;
    case SEEK_CUR:
        stream->seek(stream->position() + offset);
        break;
    case SEEK_END:
        stream->seek(stream->size() + offset);
        break;
    default:
        assert(false);
        return AVERROR(EIO);
    }

    return stream->position();
}

FFmpegBlobInputStream::FFmpegBlobInputStream(Blob blob) {
    open(std::move(blob));
}

FFmpegBlobInputStream::~FFmpegBlobInputStream() {
    closeInternal();
}

void FFmpegBlobInputStream::open(Blob blob) {
    closeInternal();
    BlobInputStream::open(std::move(blob));

    unsigned char* buffer = static_cast<unsigned char*>(av_malloc(AV_INPUT_BUFFER_MIN_SIZE));
    _ctx = avio_alloc_context(buffer, AV_INPUT_BUFFER_MIN_SIZE, 0, this, &ffRead, nullptr, &ffSeek);
}

void FFmpegBlobInputStream::seek(ssize_t pos) {
    assert(_ctx);

    avio_seek(_ctx, pos, SEEK_SET); // This will call BlobInputStream::seek.
}

void FFmpegBlobInputStream::close() {
    closeInternal();
    BlobInputStream::close();
}

void FFmpegBlobInputStream::closeInternal() {
    if (!_ctx)
        return;

    av_free(_ctx->buffer);
    av_free(_ctx);
    _ctx = nullptr;
}
