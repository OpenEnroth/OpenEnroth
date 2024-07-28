#include "OpenALTrack16.h"

#include <memory>
#include <utility>

#include "Media/AudioBufferDataSource.h"

#include "Library/Logger/Logger.h"

#include "OpenALSoundProvider.h"

OpenALTrack16::OpenALTrack16() {
    al_format = AL_FORMAT_STEREO16;
    al_source = -1;
    al_sample_rate = 0;
    uiReservedData = 0;
    uiReservedDataMinimum = 0;
}

OpenALTrack16::~OpenALTrack16() { Close(); }

bool OpenALTrack16::Open(PAudioDataSource data_source) {
    pDataSource = data_source;
    if (!pDataSource) {
        return false;
    }

    if (!pDataSource->Open()) {
        return false;
    }

    alGenSources((ALuint)1, &al_source);
    if (checkOpenALError()) {
        return false;
    }

    setSourceDefaults(al_source);
    alSourcei(al_source, AL_SOURCE_RELATIVE, AL_TRUE);

    al_sample_rate = pDataSource->GetSampleRate();

    uiReservedData = 0;
    uiReservedDataMinimum = 1000 * (al_sample_rate / 1000) * 4;

    return true;
}

bool OpenALTrack16::IsValid() { return (alIsSource(al_source) != 0); }

bool OpenALTrack16::Play() {
    if (!IsValid()) {
        return false;
    }

    ALint status;
    alGetSourcei(al_source, AL_SOURCE_STATE, &status);
    if (status == AL_PLAYING) {
        return true;
    }

    if (!Update()) {
        Close();
        return false;
    }

    alSourcePlay(al_source);
    if (checkOpenALError()) {
        return false;
    }

    updater.Start(10, [this]() { Update(); });

    return true;
}

bool OpenALTrack16::Stop() {
    if (!IsValid()) {
        return false;
    }

    Close();

    return true;
}

bool OpenALTrack16::Pause() {
    if (!IsValid()) {
        return false;
    }

    alSourcePause(al_source);
    if (checkOpenALError()) {
        return false;
    }

    updater.Stop();

    return true;
}

bool OpenALTrack16::Resume() { return Play(); }

bool OpenALTrack16::SetVolume(float volume) {
    if (!IsValid()) {
        return false;
    }

    alSourcef(al_source, AL_GAIN, volume);
    if (checkOpenALError()) {
        return false;
    }

    return true;
}

float OpenALTrack16::GetVolume() {
    if (!IsValid()) {
        return 0.f;
    }

    ALfloat volume = 0.f;
    alGetSourcef(al_source, AL_GAIN, &volume);
    if (checkOpenALError()) {
        volume = 0.f;
    }
    return volume;
}

void OpenALTrack16::Close() {
    updater.Stop();
    if (pDataSource) {
        pDataSource->Close();
    }

    if (alIsSource(al_source) != 0) {
        alSourceStop(al_source);
        checkOpenALError();
        alDeleteSources(1, &al_source);
        checkOpenALError();
    }

    al_source = -1;
    al_sample_rate = 0;
    uiReservedData = 0;
    uiReservedDataMinimum = 0;
}

void OpenALTrack16::DrainBuffers() {
    ALint num_processed_buffers = 0;
    alGetSourcei(al_source, AL_BUFFERS_PROCESSED, &num_processed_buffers);
    if (num_processed_buffers <= 0) {
        return;
    }

    ALuint *processed_buffer_ids = new ALuint[num_processed_buffers];
    if (checkOpenALError()) {
        logger->warning("OpenAL: Failed to get played buffers");
    } else {
        for (ALint i = 0; i < num_processed_buffers; i++) {
            ALuint buffer = processed_buffer_ids[i];
            alSourceUnqueueBuffers(al_source, 1, &buffer);
            if (checkOpenALError()) {
                logger->warning("OpenAL: Failed to unqueue played buffer");
            } else {
                ALint size = 0;
                alGetBufferi(buffer, AL_SIZE, &size);
                uiReservedData -= size;
                alDeleteBuffers(1, &buffer);
                if (checkOpenALError()) {
                    logger->warning("OpenAL: Failed to delete played buffer");
                }
            }
        }
    }
    delete[] processed_buffer_ids;
}

bool OpenALTrack16::Update() {
    DrainBuffers();

    while (uiReservedData < uiReservedDataMinimum) {
        Blob buffer = pDataSource->GetNextBuffer();

        if (!buffer) {
            pDataSource->Close();
            if (!pDataSource->Open())
                return false;

            buffer = pDataSource->GetNextBuffer();
            if (!buffer)
                return false;
        }

        ALuint al_buffer = -1;
        alGenBuffers(1, &al_buffer);
        if (checkOpenALError()) {
            return false;
        }

        alBufferData(al_buffer, al_format, buffer.data(), buffer.size(), al_sample_rate);
        if (checkOpenALError()) {
            return false;
        }

        alSourceQueueBuffers(al_source, 1, &al_buffer);
        if (checkOpenALError()) {
            return false;
        }

        uiReservedData += buffer.size();
    }

    return true;
}

PAudioTrack CreateAudioTrack(Blob data) {
    PAudioTrack track = std::make_shared<OpenALTrack16>();

    PAudioDataSource source = CreateAudioBufferDataSource(std::move(data));
    if (!track->Open(source)) {
        track = nullptr;
    }

    return track;
}
