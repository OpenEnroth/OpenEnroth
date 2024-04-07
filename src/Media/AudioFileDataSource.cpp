#include "AudioFileDataSource.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include "Library/Logger/Logger.h"

AudioFileDataSource::AudioFileDataSource(std::string_view file_name) {
    sFileName = file_name;
}

bool AudioFileDataSource::Open() {
    if (bOpened) {
        return true;
    }

    // Open audio file
    if (avformat_open_input(&pFormatContext, sFileName.c_str(), nullptr,
                            nullptr) < 0) {
        logger->warning("ffmpeg: Unable to open input file");
        return false;
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatContext, 0, sFileName.c_str(), 0);

    return AudioBaseDataSource::Open();
}

PAudioDataSource CreateAudioFileDataSource(std::string_view file_name) {
    std::shared_ptr<AudioFileDataSource> source =
        std::make_shared<AudioFileDataSource>(file_name);
    return std::dynamic_pointer_cast<IAudioDataSource, AudioFileDataSource>(
        source);
}

