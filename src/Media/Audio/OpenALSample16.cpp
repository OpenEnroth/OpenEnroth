#include "OpenALSample16.h"

#include <memory>

#include "OpenALSoundProvider.h"
#include "OpenALAudioDataSource.h"

AudioSample16::~AudioSample16() { Close(); }

void AudioSample16::Close() {
    pDataSource = nullptr;

    if (alIsSource(al_source) != 0) {
        alSourceStop(al_source);
        checkOpenALError();
        alSourcei(al_source, AL_BUFFER, 0);
        checkOpenALError();
        alDeleteSources(1, &al_source);
        checkOpenALError();
    }

    al_source = -1;
}

void AudioSample16::defaultSource() {
    assert((al_source != -1) && "Cannot default invalid source!");
    setSourceDefaults(al_source);
    alSourcef(al_source, AL_REFERENCE_DISTANCE, REFERENCE_DIST);
    alSourcef(al_source, AL_MAX_DISTANCE, MAX_SOUND_DIST);
    alSourcef(al_source, AL_ROLLOFF_FACTOR, ROLLOFF_FACTOR);
}

bool AudioSample16::Open(PAudioDataSource data_source) {
    pDataSource = data_source;
    if (!pDataSource) {
        return false;
    }

    std::shared_ptr<OpenALAudioDataSource> openalDataSource = std::dynamic_pointer_cast<OpenALAudioDataSource, IAudioDataSource>(pDataSource);

    if (!openalDataSource->Open()) {
        return false;
    }

    alGenSources(1, &al_source);

    // For some obscure reason we sometimes get al_source == -1 even though checkOpenALError() returns false. So we check it too.
    if (checkOpenALError() || al_source == -1) {
        return false;
    }

    defaultSource();

    if (!openalDataSource->linkSource(al_source)) {
        Close();
        return false;
    }

    return true;
}

bool AudioSample16::SetPosition(float x, float y, float z, float max_dist) {
    _position = Vec3f(x, y, z);
    _maxDistance = max_dist;

    if (IsValid()) {
        alSource3f(al_source, AL_POSITION, x, y, z);
        alSourcef(al_source, AL_MAX_DISTANCE, max_dist);
        if (checkOpenALError()) {
            return false;
        }
    }

    return true;
}

bool AudioSample16::IsValid() {
    return (alIsSource(al_source) != 0);
}

bool AudioSample16::IsStopped() {
    ALint status;

    alGetSourcei(al_source, AL_SOURCE_STATE, &status);
    return status == AL_STOPPED;
}

bool AudioSample16::Play(bool loop, bool positioned) {
    if (!IsValid()) {
        return false;
    }

    alSourcei(al_source, AL_SOURCE_RELATIVE, positioned ? AL_FALSE : AL_TRUE);
    if (!positioned) {
        alSource3f(al_source, AL_POSITION, 0.f, 0.f, 0.f);
        alSourcef(al_source, AL_MAX_DISTANCE, 2000.f);
    } else {
        alSource3f(al_source, AL_POSITION, _position.x, _position.y, _position.z);
        alSourcef(al_source, AL_MAX_DISTANCE, _maxDistance);
    }
    alSource3f(al_source, AL_VELOCITY, 0.f, 0.f, 0.f);
    alSourcef(al_source, AL_GAIN, _volume);
    alSourcei(al_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);

    ALint status;
    alGetSourcei(al_source, AL_SOURCE_STATE, &status);
    if (status == AL_PLAYING) {
        return true;
    }

    alSourcePlay(al_source);
    if (checkOpenALError()) {
        return false;
    }

    return true;
}

bool AudioSample16::Stop() {
    if (!IsValid()) {
        return false;
    }

    alSourceStop(al_source);
    if (checkOpenALError()) {
        return false;
    }

    return true;
}

bool AudioSample16::Pause() {
    if (!IsValid()) {
        return false;
    }

    ALint status;
    alGetSourcei(al_source, AL_SOURCE_STATE, &status);
    if (status != AL_PLAYING) {
        return false;
    }

    alSourcePause(al_source);
    if (checkOpenALError()) {
        return false;
    }

    return true;
}

bool AudioSample16::Resume() {
    if (!IsValid()) {
        return false;
    }

    ALint status;
    alGetSourcei(al_source, AL_SOURCE_STATE, &status);
    if (status != AL_PAUSED) {
        return false;
    }

    alSourcePlay(al_source);
    if (checkOpenALError()) {
        return false;
    }

    return true;
}

bool AudioSample16::SetVolume(float volume) {
    _volume = volume;

    if (IsValid()) {
        alSourcef(al_source, AL_GAIN, volume);
        if (checkOpenALError()) {
            return false;
        }
    }

    return true;
}

PAudioSample CreateAudioSample() {
    return std::make_shared<AudioSample16>();
}
