#include "AudioBufferDataSource.h"

#include <algorithm>
#include <utility>
#include <memory>

extern "C" {
#include <libavformat/avformat.h> // NOLINT: not a C system header.
}

#include "Library/Logger/Logger.h"

AudioBufferDataSource::AudioBufferDataSource(Blob buffer) : stream(std::move(buffer)) {}

bool AudioBufferDataSource::Open() {
    if (bOpened) {
        return true;
    }

    pFormatContext = avformat_alloc_context();
    if (pFormatContext == nullptr) {
        return false;
    }

    stream.seek(0); // Reset stream to start if already played.
    pFormatContext->pb = stream.ioContext();
    pFormatContext->flags |= AVFMT_FLAG_CUSTOM_IO; // Otherwise AVIOContext will be closed in avformat_close_input.

    // Open audio file
    if (avformat_open_input(&pFormatContext, stream.displayPath().c_str(), nullptr, nullptr) < 0) {
        logger->warning("ffmpeg: Unable to open input buffer");
        return false;
    }

    // Dump information about buffer onto standard error
    av_dump_format(pFormatContext, 0, nullptr, 0);

    return AudioBaseDataSource::Open();
}

PAudioDataSource CreateAudioBufferDataSource(Blob buffer) {
    return std::make_shared<AudioBufferDataSource>(std::move(buffer));
}
