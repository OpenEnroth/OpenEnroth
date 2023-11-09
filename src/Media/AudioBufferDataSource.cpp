#include "AudioBufferDataSource.h"

#include <algorithm>
#include <utility>

extern "C" {
#include <libavformat/avformat.h> // NOLINT: not a C system header.
}

#include "Library/Logger/Logger.h"

AudioBufferDataSource::AudioBufferDataSource(Blob buffer) : buffer(std::move(buffer)) {
    buf_pos = nullptr;
    buf_end = nullptr;
    avio_ctx_buffer = nullptr;
    avio_ctx_buffer_size = 4096;
    avio_ctx = nullptr;
}

bool AudioBufferDataSource::Open() {
    if (bOpened) {
        return true;
    }

    pFormatContext = avformat_alloc_context();
    if (pFormatContext == nullptr) {
        return false;
    }

    avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
    if (avio_ctx_buffer == nullptr) {
        Close();
        return false;
    }

    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0,
                                  this, &read_packet, nullptr, &seek);
    if (!avio_ctx) {
        Close();
        return false;
    }

    pFormatContext->pb = avio_ctx;

    buf_pos = static_cast<const uint8_t *>(buffer.data());
    buf_end = buf_pos + buffer.size();

    // Open audio file
    if (avformat_open_input(&pFormatContext, nullptr, nullptr, nullptr) < 0) {
        logger->warning("ffmpeg: Unable to open input buffer");
        return false;
    }

    // Dump information about buffer onto standard error
    av_dump_format(pFormatContext, 0, nullptr, 0);

    return AudioBaseDataSource::Open();
}

int AudioBufferDataSource::read_packet(void *opaque, uint8_t *buf,
                                       int buf_size) {
    AudioBufferDataSource *pThis = (AudioBufferDataSource *)opaque;
    return pThis->ReadPacket(buf, buf_size);
}

int AudioBufferDataSource::ReadPacket(uint8_t *buf, int buf_size) {
    int size = buf_end - buf_pos;
    if (size <= 0) {
        return AVERROR_EOF;
    }
    size = std::min(buf_size, size);
    memcpy(buf, buf_pos, size);
    buf_pos += size;
    return size;
}

int64_t AudioBufferDataSource::seek(void *opaque, int64_t offset, int whence) {
    AudioBufferDataSource *pThis = (AudioBufferDataSource *)opaque;
    return pThis->Seek(opaque, offset, whence);
}

int64_t AudioBufferDataSource::Seek(void *opaque, int64_t offset, int whence) {
    if ((whence & AVSEEK_SIZE) == AVSEEK_SIZE) {
        return buffer.size();
    }
    int force = whence & AVSEEK_FORCE;
    whence &= ~AVSEEK_FORCE;
    whence &= ~AVSEEK_SIZE;
    const uint8_t *buf_start = static_cast<const uint8_t *>(buffer.data());
    if (whence == SEEK_SET) {
        buf_pos = std::clamp(buf_start + offset, buf_start, buf_end);
        return buf_pos - buf_start;
    } else if (whence == SEEK_CUR) {
        buf_pos = std::clamp(buf_pos + offset, buf_start, buf_end);
        return buf_pos - buf_start;
    } else if (whence == SEEK_END) {
        buf_pos = std::clamp(buf_end + offset, buf_start, buf_end);
        return buf_pos - buf_start;
    }
    return AVERROR(EIO);
}

PAudioDataSource CreateAudioBufferDataSource(Blob buffer) {
    return std::make_shared<AudioBufferDataSource>(std::move(buffer));
}
