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
    FFmpegBlobInputStream *stream = static_cast<FFmpegBlobInputStream *>(opaque);

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
    BlobInputStream::open(std::move(blob));
    resetContext();
}

void FFmpegBlobInputStream::close() {
    BlobInputStream::close();
    closeInternal();
}

void FFmpegBlobInputStream::closeInternal() {
    if (!_ctx)
        return;

    av_free(_ctx->buffer);
    av_free(_ctx);
    _ctx = nullptr;
}

void FFmpegBlobInputStream::resetContext() {
    closeInternal();
    unsigned char* buffer = static_cast<unsigned char*>(av_malloc(AV_INPUT_BUFFER_MIN_SIZE));
    _ctx = avio_alloc_context(buffer, AV_INPUT_BUFFER_MIN_SIZE, 0, this, &ffRead, nullptr, &ffSeek);
}
