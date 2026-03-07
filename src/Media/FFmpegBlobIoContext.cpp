#include "FFmpegBlobIoContext.h"

#include <algorithm>
#include <cassert>
#include <utility>

extern "C" {
#include <libavcodec/avcodec.h> // NOLINT: not a C system header.
#include <libavformat/avio.h> // NOLINT: not a C system header.
#include <libavutil/mem.h> // NOLINT: not a C system header.
}

int ffRead(void *opaque, uint8_t *buf, int size) {
    FFmpegBlobIoContext *ctx = static_cast<FFmpegBlobIoContext *>(opaque);
    size_t bytes = ctx->_stream.read(buf, static_cast<size_t>(size));
    if (bytes == 0)
        return AVERROR_EOF;
    return bytes;
}

int64_t ffSeek(void *opaque, int64_t offset, int whence) {
    FFmpegBlobIoContext *ctx = static_cast<FFmpegBlobIoContext *>(opaque);

    if (whence & AVSEEK_SIZE)
        return ctx->_blob.size();

    size_t pos = ctx->_stream.position();

    whence &= ~AVSEEK_FORCE;
    switch (whence) {
    case SEEK_SET:
        pos = std::clamp<size_t>(offset, 0, ctx->_blob.size());
        break;
    case SEEK_CUR:
        pos = std::clamp<size_t>(pos + offset, 0, ctx->_blob.size());
        break;
    case SEEK_END:
        pos = std::clamp<size_t>(ctx->_blob.size() + offset, 0, ctx->_blob.size());
        break;
    default:
        assert(false);
        return AVERROR(EIO);
    }

    // Reopen stream and skip to the new position.
    ctx->_stream.close();
    ctx->_stream.open(ctx->_blob);
    return ctx->_stream.skip(pos);
}

FFmpegBlobIoContext::FFmpegBlobIoContext(Blob blob) {
    reset(std::move(blob));
}

FFmpegBlobIoContext::~FFmpegBlobIoContext() {
    destroyAvioContext();
}

void FFmpegBlobIoContext::reset(Blob blob) {
    destroyAvioContext();
    _blob = std::move(blob);
    _stream.open(_blob);
    createAvioContext();
}

void FFmpegBlobIoContext::createAvioContext() {
    unsigned char *buffer = static_cast<unsigned char *>(av_malloc(AV_INPUT_BUFFER_MIN_SIZE));
    _ctx = avio_alloc_context(buffer, AV_INPUT_BUFFER_MIN_SIZE, 0, this, &ffRead, nullptr, &ffSeek);
}

void FFmpegBlobIoContext::destroyAvioContext() {
    if (!_ctx)
        return;

    av_free(_ctx->buffer);
    av_free(_ctx);
    _ctx = nullptr;
    _stream.close();
}
