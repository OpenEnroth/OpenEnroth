#include "AudioBufferDataSource.h"

#include <algorithm>
#include <utility>
#include <memory>

extern "C" {
#include <libavformat/avformat.h> // NOLINT: not a C system header.
}

#include "Library/Logger/Logger.h"

AudioBufferDataSource::AudioBufferDataSource(Blob buffer) : _ioContext(std::move(buffer)) {}

bool AudioBufferDataSource::Open() {
    if (bOpened)
        return true;

    pFormatContext = avformat_alloc_context();
    if (pFormatContext == nullptr)
        return false;

    // Give every open a fresh AVIOContext over the same blob. A reopen after avformat_close_input (e.g. a looping
    // music track) must not reuse the old one: avio_seek keeps the closed demuxer's checksum state, and the next
    // fill_buffer runs av_crc through a stale pointer (SIGSEGV in the mp3 id3v1 probe).
    _ioContext.reset(Blob::share(_ioContext.blob()));

    pFormatContext->pb = _ioContext.avioContext();

    // Open audio file
    if (avformat_open_input(&pFormatContext, _ioContext.blob().displayPath().c_str(), nullptr, nullptr) < 0) {
        MM_WARNING("ffmpeg: Unable to open input buffer");
        return false;
    }

    // Dump information about buffer onto standard error
    av_dump_format(pFormatContext, 0, nullptr, 0);

    return AudioBaseDataSource::Open();
}

PAudioDataSource CreateAudioBufferDataSource(Blob buffer) {
    return std::make_shared<AudioBufferDataSource>(std::move(buffer));
}
