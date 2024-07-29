#include "OpenALAudioDataSource.h"

#include <memory>

#include "Library/Logger/Logger.h"

#include "OpenALSoundProvider.h"

OpenALAudioDataSource::~OpenALAudioDataSource() {
    _baseDataSource->Close();
    if (_buffers.size()) {
        alDeleteBuffers(_buffers.size(), &_buffers.front());
    }
}

bool OpenALAudioDataSource::Open() {
    if (_buffers.size()) {
        return true;
    }

    _baseDataSource->Open();

    bool result = true;
    ALsizei al_sample_rate = GetSampleRate();
    ALenum al_format = AL_FORMAT_STEREO16;
    unsigned int num_channels = GetChannelCount();

    switch (num_channels) {
        case 1:
            al_format = AL_FORMAT_MONO16;
            break;
        case 2:
            al_format = AL_FORMAT_STEREO16;
            break;
        default:
            if (alIsExtensionPresent("AL_EXT_MCFORMATS")) {
                switch (num_channels) {
                    case 4:
                        al_format = alGetEnumValue("AL_FORMAT_QUAD16");
                        break;
                    case 6:
                        al_format = alGetEnumValue("AL_FORMAT_51CHN16");
                        break;
                    case 7:
                        al_format = alGetEnumValue("AL_FORMAT_61CHN16");
                        break;
                    case 8:
                        al_format = alGetEnumValue("AL_FORMAT_71CHN16");
                        break;
                }
            }
            logger->error("Unsupported number of audio channels: {}", num_channels);
    }

    while (true) {
        Blob buffer = _baseDataSource->GetNextBuffer();
        if (!buffer) {
            break;
        }

        ALuint al_buffer = -1;
        alGenBuffers(1, &al_buffer);
        if (checkOpenALError()) {
            result = false;
            break;
        }

        alBufferData(al_buffer, al_format, buffer.data(), buffer.size(), al_sample_rate);
        if (checkOpenALError()) {
            result = false;
            break;
        }

        _buffers.push_back(al_buffer);
    }

    _baseDataSource->Close();

    return result;
}

void OpenALAudioDataSource::Close() {
    _baseDataSource->Close();
}

bool OpenALAudioDataSource::linkSource(ALuint al_source) {
    alSourceQueueBuffers(al_source, _buffers.size(), &_buffers.front());
    if (checkOpenALError()) {
        return false;
    }
    return true;
}

PAudioDataSource PlatformDataSourceInitialize(PAudioDataSource baseDataSource) {
    return std::make_shared<OpenALAudioDataSource>(baseDataSource);
}
