#include "FFmpegBlobIoContext.h"

#include <algorithm>
#include <cstring>
#include <cassert>
#include <utility>

extern "C" {
#include <libavcodec/avcodec.h> // NOLINT: not a C system header.
#include <libavformat/avio.h> // NOLINT: not a C system header.
#include <libavutil/mem.h> // NOLINT: not a C system header.
}

int ffRead(void *opaque, uint8_t *buf, int size) {
    FFmpegBlobIoContext *ctx = static_cast<FFmpegBlobIoContext *>(opaque);
    size_t bytes = std::min(static_cast<size_t>(size), ctx->_blob.size() - ctx->_pos);
    if (bytes == 0)
        return AVERROR_EOF;
    memcpy(buf, static_cast<const char *>(ctx->_blob.data()) + ctx->_pos, bytes);
    ctx->_pos += bytes;
    return bytes;
}

int64_t ffSeek(void *opaque, int64_t offset, int whence) {
    FFmpegBlobIoContext *ctx = static_cast<FFmpegBlobIoContext *>(opaque);

    if (whence & AVSEEK_SIZE)
        return ctx->_blob.size();

    whence &= ~AVSEEK_FORCE;
    switch (whence) {
    case SEEK_SET:
        ctx->_pos = std::clamp<size_t>(offset, 0, ctx->_blob.size());
        break;
    case SEEK_CUR:
        ctx->_pos = std::clamp<size_t>(ctx->_pos + offset, 0, ctx->_blob.size());
        break;
    case SEEK_END:
        ctx->_pos = std::clamp<size_t>(ctx->_blob.size() + offset, 0, ctx->_blob.size());
        break;
    default:
        assert(false);
        return AVERROR(EIO);
    }

    return ctx->_pos;
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
    _pos = 0;
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
}
