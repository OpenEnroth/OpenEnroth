#include "OpenALSoundProvider.h"

#include <cassert>
#include <cmath>

#include <alext.h> // For ALC_HRTF_SOFT. NOLINT: not a C system header.

#include "Engine/Engine.h"

#include "Library/Logger/Logger.h"

bool checkOpenALError() {
    ALenum code1 = alGetError();
    if (code1 == AL_NO_ERROR) {
        return false;
    }

    const char *message = alGetString(code1);
    logger->warning("OpenAL: error #{} \"{}\"", code1, message ? message : "");

    return true;
}

void setSourceDefaults(ALuint al_source) {
    alSourcei(al_source, AL_LOOPING, AL_FALSE);
    alSourcef(al_source, AL_PITCH, 1.f);
    alSourcef(al_source, AL_GAIN, 1.f);
    alSource3f(al_source, AL_POSITION, 0.f, 0.f, 0.f);
    alSource3f(al_source, AL_VELOCITY, 0.f, 0.f, 0.f);
}

OpenALSoundProvider::OpenALSoundProvider() {
    device = nullptr;
    context = nullptr;
}

OpenALSoundProvider::~OpenALSoundProvider() { Release(); }

bool OpenALSoundProvider::Initialize() {
    const char *device_names = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
    if (!device_names) {
        device_names = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
    }

    if (device_names) {
        for (const char *device_name = device_names; device_name[0]; device_name += strlen(device_name) + 1) {
            logger->info("OpenAL: device found \"{}\"", device_name);
        }
    }

    const char *defname = alcGetString(0, ALC_DEFAULT_DEVICE_SPECIFIER);

    device = alcOpenDevice(defname);
    if (device == nullptr) {
        checkOpenALError();
        logger->warning("OpenAL: Default sound device not present");
        return false;
    }

    ALCint attrs[3] = {};

    if (engine->config->audio.DisableHRTF.value()) {
        // Disable HRTF so headphones don't sound bad.
        attrs[0] = ALC_HRTF_SOFT;
        attrs[1] = ALC_FALSE;
        attrs[2] = 0; // end of list
    }

    context = alcCreateContext(device, attrs);
    if (context == nullptr) {
        checkOpenALError();
        Release();
        return false;
    }

    if (alcMakeContextCurrent(context) != ALC_TRUE) {
        checkOpenALError();
        Release();
        return false;
    }

    alListener3f(AL_POSITION, 0.f, 0.f, 0.f);
    alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f);
    ALfloat listenerOri[] = {0.f, 1.f, 0.f, 0.f, 0.f, -1.f};
    alListenerfv(AL_ORIENTATION, listenerOri);

    return true;
}

void OpenALSoundProvider::SetListenerPosition(float x, float y, float z) {
    alListener3f(AL_POSITION, x, y, z);
}

void OpenALSoundProvider::SetOrientation(float yaw, float pitch) {
    float x = std::cos(pitch) * std::cos(yaw);
    float y = std::cos(pitch) * std::sin(yaw);
    float z = -std::sin(pitch);

    ALfloat listenerOri[] = {x, y, z, 0.f, 0.f, 1.f};
    alListenerfv(AL_ORIENTATION, listenerOri);
}

void OpenALSoundProvider::Release() {
    alcMakeContextCurrent(nullptr);
    if (context) {
        alcDestroyContext(context);
    }
    if (device) {
        alcCloseDevice(device);
    }
}

void OpenALSoundProvider::DeleteBuffers(StreamingTrackBuffer *track, int type) {
    int count = 0;
    alGetSourcei(track->source_id, type, &count);
    if (checkOpenALError()) {
        logger->warning("OpenAL: Fail to get buffers count.");
        assert(false);
    }
    if (count > 0) {
        unsigned int *buffer_ids = new unsigned int[count];
        alSourceUnqueueBuffers(track->source_id, count, buffer_ids);
        if (checkOpenALError()) {
            logger->warning("OpenAL: Fail to unqueue buffers.");
            assert(false);
        } else {
            alDeleteBuffers(count, buffer_ids);
            if (checkOpenALError()) {
                logger->warning("OpenAL: Fail to delete buffers.");
                assert(false);
            }
        }
        delete[] buffer_ids;
    }
}

void OpenALSoundProvider::DeleteStreamingTrack(StreamingTrackBuffer **buffer) {
    if (!buffer && !*buffer) return;
    StreamingTrackBuffer *track = *buffer;

    int status;
    alGetSourcei(track->source_id, AL_SOURCE_STATE, &status);
    if (status == AL_PLAYING) {
        alSourceStop(track->source_id);
        if (checkOpenALError()) assert(false);
    }
    alSourcei(track->source_id, AL_LOOPING, AL_FALSE);
    if (checkOpenALError()) {
        logger->warning("OpenAL: Fail to disable looping.");
        assert(false);
    }

    DeleteBuffers(track, AL_BUFFERS_PROCESSED);
    DeleteBuffers(track, AL_BUFFERS_QUEUED);

    alDeleteSources(1, &track->source_id);
    if (checkOpenALError()) assert(false);

    delete *buffer;
    *buffer = nullptr;
}

void OpenALSoundProvider::DeleteBuffer16(TrackBuffer **buffer) {
    alDeleteBuffers(1, &(*buffer)->buffer_id);
    checkOpenALError();

    delete *buffer;
    *buffer = nullptr;
}

float OpenALSoundProvider::alBufferLength(unsigned int buffer) {
    int size, bits, channels, freq;

    alGetBufferi(buffer, AL_SIZE, &size);
    alGetBufferi(buffer, AL_BITS, &bits);
    alGetBufferi(buffer, AL_CHANNELS, &channels);
    alGetBufferi(buffer, AL_FREQUENCY, &freq);
    if (checkOpenALError()) {
        return 0.f;
    }

    return (ALfloat)((ALuint)size / channels / (bits / 8)) / (ALfloat)freq;
}

OpenALSoundProvider::StreamingTrackBuffer *
OpenALSoundProvider::CreateStreamingTrack16(int num_channels, int sample_rate,
                                            int bytes_per_sample) {
    assert(bytes_per_sample == 2 && "OpenALSoundProvider: unsupported sample size");

    ALenum sound_format;
    switch (num_channels) {
        case 1:
            sound_format = AL_FORMAT_MONO16;
            break;
        case 2:
            sound_format = AL_FORMAT_STEREO16;
            break;
        default:
            if (alIsExtensionPresent("AL_EXT_MCFORMATS")) {
                switch (num_channels) {
                    case 4:
                        sound_format = alGetEnumValue("AL_FORMAT_QUAD16");
                        break;
                    case 6:
                        sound_format = alGetEnumValue("AL_FORMAT_51CHN16");
                        break;
                    case 7:
                        sound_format = alGetEnumValue("AL_FORMAT_61CHN16");
                        break;
                    case 8:
                        sound_format = alGetEnumValue("AL_FORMAT_71CHN16");
                        break;
                }
            }
            logger->error("Unsupported number of audio channels: {}", num_channels);
    }

    unsigned int al_source = -1;
    alGenSources(1, &al_source);
    if (checkOpenALError()) {
        return nullptr;
    }

    setSourceDefaults(al_source);

    StreamingTrackBuffer *ret = new StreamingTrackBuffer;
    ret->source_id = al_source;
    ret->sample_format = sound_format;
    ret->sample_rate = sample_rate;
    return ret;
}

void OpenALSoundProvider::Stream16(StreamingTrackBuffer *buffer,
                                   int num_samples, const void *samples,
                                   bool wait) {
    if (buffer == nullptr) {
        return;
    }

    int bytes_per_sample = 2;

    DeleteBuffers(buffer, AL_BUFFERS_PROCESSED);

    unsigned int al_buffer;
    alGenBuffers(1, &al_buffer);
    alBufferData(al_buffer, buffer->sample_format, samples,
                 num_samples * bytes_per_sample, buffer->sample_rate);
    if (checkOpenALError()) {
        alDeleteBuffers(1, &al_buffer);
        return;
    }

    alSourceQueueBuffers(buffer->source_id, 1, &al_buffer);
    if (checkOpenALError()) {
        alDeleteBuffers(1, &al_buffer);
        return;
    }

    volatile int status;
    alGetSourcei(buffer->source_id, AL_SOURCE_STATE, (int *)&status);
    if (status != AL_PLAYING) {
        alSourcePlay(buffer->source_id);
        if (checkOpenALError()) {
            assert(false);
        }

        if (wait) {
            do {
                alGetSourcei(buffer->source_id, AL_SOURCE_STATE,
                             (int *)&status);
            } while (status == AL_PLAYING);
        }
    }
}

OpenALSoundProvider::TrackBuffer *OpenALSoundProvider::CreateTrack16(
    int num_channels, int sample_rate, const void *data, size_t size) {
    ALenum sound_format;
    switch (num_channels) {
        case 1: {
            sound_format = AL_FORMAT_MONO16;
            break;
        }
        case 2: {
            sound_format = AL_FORMAT_STEREO16;
            break;
        }
        default: {
            if (alIsExtensionPresent("AL_EXT_MCFORMATS")) {
                switch (num_channels) {
                    case 4:
                        sound_format = alGetEnumValue("AL_FORMAT_QUAD16");
                        break;
                    case 6:
                        sound_format = alGetEnumValue("AL_FORMAT_51CHN16");
                        break;
                    case 7:
                        sound_format = alGetEnumValue("AL_FORMAT_61CHN16");
                        break;
                    case 8:
                        sound_format = alGetEnumValue("AL_FORMAT_71CHN16");
                        break;
                }
            }
            logger->error("Unsupported number of audio channels: {}", num_channels);
        }
    }

    ALuint al_source = -1;
    alGenSources((ALuint)1, &al_source);
    if (checkOpenALError()) {
        return nullptr;
    }

    setSourceDefaults(al_source);

    ALuint al_buffer = -1;
    alGenBuffers(1, &al_buffer);
    if (checkOpenALError()) {
        alDeleteSources(1, &al_source);
        return nullptr;
    }

    alBufferData(al_buffer, sound_format, data, size, sample_rate);
    if (checkOpenALError()) {
        alDeleteSources(1, &al_source);
        alDeleteBuffers(1, &al_buffer);
        return nullptr;
    }

    alSourcei(al_source, AL_BUFFER, al_buffer);
    if (checkOpenALError()) {
        alDeleteSources(1, &al_source);
        alDeleteBuffers(1, &al_buffer);
        return nullptr;
    }

    TrackBuffer *ret = new TrackBuffer;
    ret->source_id = al_source;
    ret->buffer_id = al_buffer;
    return ret;
}

void OpenALSoundProvider::PlayTrack16(TrackBuffer *buffer, bool loop,
                                      bool wait) {
    int status;
    alGetSourcei(buffer->source_id, AL_SOURCE_STATE, (int *)&status);
    if (status == AL_PLAYING) {
        return;
    }

    alSourcei(buffer->source_id, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcePlay(buffer->source_id);
    if (checkOpenALError()) {
        assert(false);
    }

    if (wait && !loop) {
        float track_length = alBufferLength(buffer->buffer_id);
        do {
            float track_offset = 0;
            alGetSourcef(buffer->source_id, AL_SEC_OFFSET, &track_offset);
            logger->info("OpenAL: playing {:.4f}/{:.4f}\n", track_offset, track_length);

            alGetSourcei(buffer->source_id, AL_SOURCE_STATE, (int *)&status);
        } while (status == AL_PLAYING);
    }
}

