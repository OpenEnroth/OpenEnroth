#include "AudioBaseDataSource.h"

#include <utility>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h> // NOLINT: not a C system header.
#include <libavformat/avformat.h> // NOLINT: not a C system header.
#include <libswresample/swresample.h> // NOLINT: not a C system header.
}

#include "Library/Logger/Logger.h"

AudioBaseDataSource::AudioBaseDataSource() {
    pFormatContext = nullptr;
    iStreamIndex = -1;
    pCodecContext = nullptr;
    pConverter = nullptr;
    bOpened = false;

    _savedDuration = 0.0f;
}

bool AudioBaseDataSource::Open() {
    // Retrieve stream information
    if (avformat_find_stream_info(pFormatContext, nullptr) < 0) {
        Close();
        logger->warning("ffmpeg: Unable to find stream info");
        return false;
    }

#if LIBAVFORMAT_VERSION_MAJOR >= 59
    const AVCodec *codec = nullptr;
#else
    AVCodec *codec = nullptr;
#endif
    iStreamIndex = av_find_best_stream(pFormatContext, AVMEDIA_TYPE_AUDIO, -1,
                                       -1, &codec, 0);
    if (iStreamIndex < 0) {
        Close();
        logger->warning("ffmpeg: Unable to find audio stream");
        return false;
    }

    AVStream *stream = pFormatContext->streams[iStreamIndex];
    pCodecContext = avcodec_alloc_context3(codec);
    if (pCodecContext == nullptr) {
        Close();
        return false;
    }

    if (avcodec_parameters_to_context(pCodecContext, stream->codecpar) < 0) {
        Close();
        return false;
    }
    if (avcodec_open2(pCodecContext, codec, nullptr) < 0) {
        Close();
        return false;
    }

    int status = swr_alloc_set_opts2(
        &pConverter,
        &pCodecContext->ch_layout,
        AV_SAMPLE_FMT_S16, pCodecContext->sample_rate,
        &pCodecContext->ch_layout,
        pCodecContext->sample_fmt, pCodecContext->sample_rate, 0, nullptr);
    if (status < 0) {
        Close();
        logger->warning("ffmpeg: Failed to set converter options");
        return false;
    }
    if (swr_init(pConverter) < 0) {
        Close();
        logger->warning("ffmpeg: Failed to create converter");
        return false;
    }

    bOpened = true;
    _savedDuration = static_cast<float>(pFormatContext->duration) / AV_TIME_BASE;

    return true;
}

void AudioBaseDataSource::Close() {
    if (pConverter != nullptr) {
        swr_free(&pConverter);
        pConverter = nullptr;
    }

    if (pCodecContext) {
        avcodec_close(pCodecContext);
        pCodecContext = nullptr;
    }

    iStreamIndex = -1;

    if (pFormatContext != nullptr) {
        avformat_close_input(&pFormatContext);
        pFormatContext = nullptr;
    }

    bOpened = false;
}

size_t AudioBaseDataSource::GetSampleRate() {
    if (pCodecContext == nullptr) {
        return 0;
    }

    return pCodecContext->sample_rate;
}

size_t AudioBaseDataSource::GetChannelCount() {
    if (pCodecContext == nullptr) {
        return 0;
    }

    return pCodecContext->ch_layout.nb_channels;
}

Blob AudioBaseDataSource::GetNextBuffer() {
    Blob buffer;

    if (!queue.empty()) {
        buffer = std::move(queue.front());
        queue.pop();
    }

    AVPacket *packet = av_packet_alloc();

    if (av_read_frame(pFormatContext, packet) >= 0) {
        if (avcodec_send_packet(pCodecContext, packet) >= 0) {
            Blob result;
            AVFrame *frame = av_frame_alloc();
            int res = 0;
            while (res >= 0) {
                res = avcodec_receive_frame(pCodecContext, frame);
                if (res == AVERROR(EAGAIN) || res == AVERROR_EOF) {
                    break;
                }
                if (res < 0) {
                    return buffer;
                }
                size_t tmp_size = frame->nb_samples * pCodecContext->ch_layout.nb_channels * 2;
                std::unique_ptr<void, FreeDeleter> tmp_buf(malloc(tmp_size));
                uint8_t *dst_channels[8] = { static_cast<uint8_t *>(tmp_buf.get()) };
                int got_samples = swr_convert(
                    pConverter, dst_channels, frame->nb_samples,
                    (const uint8_t **)frame->data, frame->nb_samples);

                Blob tmp_blob = Blob::fromMalloc(std::move(tmp_buf), tmp_size);

                if (got_samples > 0) {
                    if (!buffer) {
                        buffer = std::move(tmp_blob);
                    } else {
                        queue.push(std::move(tmp_blob));
                    }
                }
            }
            av_frame_free(&frame);
        }
    }

    av_packet_free(&packet);

    return buffer;
}

float AudioBaseDataSource::GetDuration() {
    return _savedDuration;
}
