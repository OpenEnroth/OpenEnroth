#include "Media/Audio/OpenALSoundProvider.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <atomic>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <functional>
#include <utility>
#include <vector>

#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>

#include "Engine/ErrorHandling.h"
#include "Engine/EngineIocContainer.h"
#include "Library/Logger/Logger.h"
#include "Media/MediaPlayer.h"

bool CheckError() {
    ALenum code1 = alGetError();
    if (code1 == AL_NO_ERROR) {
        return false;
    }

    const char *message = alGetString(code1);
    logger->warning("OpenAL: error #{} \"{}\"", code1, message);

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
        for (const char *device_name = device_names; device_name[0];
             device_name += strlen(device_name) + 1) {
            logger->info("OpenAL: device found \"{}\"", device_name);
        }
    }

    const char *defname = alcGetString(0, ALC_DEFAULT_DEVICE_SPECIFIER);

    device = alcOpenDevice(defname);
    if (device == nullptr) {
        CheckError();
        logger->warning("OpenAL: Default sound device not present");
        return false;
    }

    context = alcCreateContext(device, nullptr);
    if (context == nullptr) {
        CheckError();
        Release();
        return false;
    }

    if (alcMakeContextCurrent(context) != ALC_TRUE) {
        CheckError();
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
    if (CheckError()) {
        logger->warning("OpenAL: Fail to get buffers count.");
        assert(false);
    }
    if (count > 0) {
        unsigned int *buffer_ids = new unsigned int[count];
        alSourceUnqueueBuffers(track->source_id, count, buffer_ids);
        if (CheckError()) {
            logger->warning("OpenAL: Fail to unqueue buffers.");
            assert(false);
        } else {
            alDeleteBuffers(count, buffer_ids);
            if (CheckError()) {
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
        if (CheckError()) assert(false);
    }
    alSourcei(track->source_id, AL_LOOPING, AL_FALSE);
    if (CheckError()) {
        logger->warning("OpenAL: Fail to disable looping.");
        assert(false);
    }

    DeleteBuffers(track, AL_BUFFERS_PROCESSED);
    DeleteBuffers(track, AL_BUFFERS_QUEUED);

    alDeleteSources(1, &track->source_id);
    if (CheckError()) assert(false);

    delete *buffer;
    *buffer = nullptr;
}

void OpenALSoundProvider::DeleteBuffer16(TrackBuffer **buffer) {
    alDeleteBuffers(1, &(*buffer)->buffer_id);
    CheckError();

    delete *buffer;
    *buffer = nullptr;
}

float OpenALSoundProvider::alBufferLength(unsigned int buffer) {
    int size, bits, channels, freq;

    alGetBufferi(buffer, AL_SIZE, &size);
    alGetBufferi(buffer, AL_BITS, &bits);
    alGetBufferi(buffer, AL_CHANNELS, &channels);
    alGetBufferi(buffer, AL_FREQUENCY, &freq);
    if (CheckError()) {
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
            Error("Unsupported number of audio channels: %u", num_channels);
    }

    unsigned int al_source = -1;
    alGenSources(1, &al_source);
    if (CheckError()) {
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
    if (CheckError()) {
        alDeleteBuffers(1, &al_buffer);
        return;
    }

    alSourceQueueBuffers(buffer->source_id, 1, &al_buffer);
    if (CheckError()) {
        alDeleteBuffers(1, &al_buffer);
        return;
    }

    volatile int status;
    alGetSourcei(buffer->source_id, AL_SOURCE_STATE, (int *)&status);
    if (status != AL_PLAYING) {
        alSourcePlay(buffer->source_id);
        if (CheckError()) {
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
            Error("Unsupported number of audio channels: %u", num_channels);
        }
    }

    ALuint al_source = -1;
    alGenSources((ALuint)1, &al_source);
    if (CheckError()) {
        return nullptr;
    }

    setSourceDefaults(al_source);

    ALuint al_buffer = -1;
    alGenBuffers(1, &al_buffer);
    if (CheckError()) {
        alDeleteSources(1, &al_source);
        return nullptr;
    }

    alBufferData(al_buffer, sound_format, data, size, sample_rate);
    if (CheckError()) {
        alDeleteSources(1, &al_source);
        alDeleteBuffers(1, &al_buffer);
        return nullptr;
    }

    alSourcei(al_source, AL_BUFFER, al_buffer);
    if (CheckError()) {
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
    if (CheckError()) {
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

// CallBackTimer

class CallBackTimer {
 public:
    CallBackTimer() : bRunning(false) {}
    virtual ~CallBackTimer() {
        if (bRunning.load(std::memory_order_acquire)) {
            Stop();
        }
    }

    void Start(int interval, std::function<void(void)> func) {
        if (bRunning.load(std::memory_order_acquire)) {
            Stop();
        }
        bRunning.store(true, std::memory_order_release);
        theThread = std::thread([this, interval, func]() {
            while (bRunning.load(std::memory_order_acquire)) {
                func();
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(interval));
            }
        });
    }

    void Stop() {
        bRunning.store(false, std::memory_order_release);
        if (theThread.joinable()) {
            theThread.join();
        }
    }

    bool IsRunning() const noexcept {
        return (bRunning.load(std::memory_order_acquire) &&
                theThread.joinable());
    }

 private:
    std::atomic<bool> bRunning;
    std::thread theThread;
};

// AudioTrack

class AudioTrackS16 : public IAudioTrack {
 public:
    AudioTrackS16();
    virtual ~AudioTrackS16();

    virtual bool Open(PAudioDataSource data_source) override;
    virtual bool IsValid() override;

    virtual bool Play() override;
    virtual bool Stop() override;
    virtual bool Pause() override;
    virtual bool Resume() override;
    virtual bool SetVolume(float volume) override;
    virtual float GetVolume() override;

 protected:
    void Close();
    void DrainBuffers();
    bool Update();

    PAudioDataSource pDataSource;
    CallBackTimer updater;
    ALenum al_format;
    ALuint al_source;
    ALsizei al_sample_rate;
    size_t uiReservedData;
    size_t uiReservedDataMinimum;
};

AudioTrackS16::AudioTrackS16() {
    al_format = AL_FORMAT_STEREO16;
    al_source = -1;
    al_sample_rate = 0;
    uiReservedData = 0;
    uiReservedDataMinimum = 0;
}

AudioTrackS16::~AudioTrackS16() { Close(); }

bool AudioTrackS16::Open(PAudioDataSource data_source) {
    pDataSource = data_source;
    if (!pDataSource) {
        return false;
    }

    if (!pDataSource->Open()) {
        return false;
    }

    alGenSources((ALuint)1, &al_source);
    if (CheckError()) {
        return false;
    }

    setSourceDefaults(al_source);
    alSourcei(al_source, AL_SOURCE_RELATIVE, AL_TRUE);

    al_sample_rate = pDataSource->GetSampleRate();

    uiReservedData = 0;
    uiReservedDataMinimum = 1000 * (al_sample_rate / 1000) * 4;

    return true;
}

bool AudioTrackS16::IsValid() { return (alIsSource(al_source) != 0); }

bool AudioTrackS16::Play() {
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
    if (CheckError()) {
        return false;
    }

    updater.Start(10, [this]() { Update(); });

    return true;
}

bool AudioTrackS16::Stop() {
    if (!IsValid()) {
        return false;
    }

    Close();

    return true;
}

bool AudioTrackS16::Pause() {
    if (!IsValid()) {
        return false;
    }

    alSourcePause(al_source);
    if (CheckError()) {
        return false;
    }

    updater.Stop();

    return true;
}

bool AudioTrackS16::Resume() { return Play(); }

bool AudioTrackS16::SetVolume(float volume) {
    if (!IsValid()) {
        return false;
    }

    alSourcef(al_source, AL_GAIN, volume);
    if (CheckError()) {
        return false;
    }

    return true;
}

float AudioTrackS16::GetVolume() {
    if (!IsValid()) {
        return 0.f;
    }

    ALfloat volume = 0.f;
    alGetSourcef(al_source, AL_GAIN, &volume);
    if (CheckError()) {
        volume = 0.f;
    }
    return volume;
}

void AudioTrackS16::Close() {
    updater.Stop();
    if (pDataSource) {
        pDataSource->Close();
    }

    if (alIsSource(al_source) != 0) {
        alSourceStop(al_source);
        CheckError();
        alDeleteSources(1, &al_source);
        CheckError();
    }

    al_source = -1;
    al_sample_rate = 0;
    uiReservedData = 0;
    uiReservedDataMinimum = 0;
}

void AudioTrackS16::DrainBuffers() {
    ALint num_processed_buffers = 0;
    alGetSourcei(al_source, AL_BUFFERS_PROCESSED, &num_processed_buffers);
    if (num_processed_buffers <= 0) {
        return;
    }

    ALuint *processed_buffer_ids = new ALuint[num_processed_buffers];
    if (CheckError()) {
        logger->warning("OpenAL: Failed to get played buffers");
    } else {
        for (ALint i = 0; i < num_processed_buffers; i++) {
            ALuint buffer = processed_buffer_ids[i];
            alSourceUnqueueBuffers(al_source, 1, &buffer);
            if (CheckError()) {
                logger->warning("OpenAL: Failed to unqueue played buffer");
            } else {
                ALint size = 0;
                alGetBufferi(buffer, AL_SIZE, &size);
                uiReservedData -= size;
                alDeleteBuffers(1, &buffer);
                if (CheckError()) {
                    logger->warning("OpenAL: Failed to delete played buffer");
                }
            }
        }
    }
    delete[] processed_buffer_ids;
}

bool AudioTrackS16::Update() {
    DrainBuffers();

    while (uiReservedData < uiReservedDataMinimum) {
        std::shared_ptr<Blob> buffer = pDataSource->GetNextBuffer();

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
        if (CheckError()) {
            return false;
        }

        alBufferData(al_buffer, al_format, buffer->data(), buffer->size(), al_sample_rate);
        if (CheckError()) {
            return false;
        }

        alSourceQueueBuffers(al_source, 1, &al_buffer);
        if (CheckError()) {
            return false;
        }

        uiReservedData += buffer->size();
    }

    return true;
}

PAudioTrack CreateAudioTrack(const std::string &file_path) {
    std::shared_ptr<AudioTrackS16> track = std::make_shared<AudioTrackS16>();

    PAudioDataSource source = CreateAudioFileDataSource(file_path);
    if (!track->Open(source)) {
        track = nullptr;
    }

    return std::dynamic_pointer_cast<IAudioTrack, AudioTrackS16>(track);
}

// TODO(Nik-RE-dev): this middleware class is temporary because Media API is not fully
// ready to properly support current use cases
class OpenALAudioDataSource : public IAudioDataSource {
 public:
    explicit OpenALAudioDataSource(PAudioDataSource baseDataSource):_baseDataSource(baseDataSource) {}
    virtual ~OpenALAudioDataSource() override;

    virtual bool Open() override;
    virtual void Close() override;

    virtual size_t GetSampleRate() override { return _baseDataSource->GetSampleRate(); }
    virtual size_t GetChannelCount() override { return _baseDataSource->GetChannelCount(); }
    virtual std::shared_ptr<Blob> GetNextBuffer() override { return _baseDataSource->GetNextBuffer(); }

    bool linkSource(ALuint al_source);

 protected:
    PAudioDataSource _baseDataSource;
    std::vector<ALuint> _buffers;
};

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
            Error("Unsupported number of audio channels: %u", num_channels);
    }

    while (true) {
        std::shared_ptr<Blob> buffer = _baseDataSource->GetNextBuffer();
        if (!buffer) {
            break;
        }

        ALuint al_buffer = -1;
        alGenBuffers(1, &al_buffer);
        if (CheckError()) {
            result = false;
            break;
        }

        alBufferData(al_buffer, al_format, buffer->data(), buffer->size(), al_sample_rate);
        if (CheckError()) {
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
    if (CheckError()) {
        return false;
    }
    return true;
}

PAudioDataSource PlatformDataSourceInitialize(PAudioDataSource baseDataSource) {
    return std::make_shared<OpenALAudioDataSource>(baseDataSource);
}

class AudioSample16 : public IAudioSample {
 public:
    AudioSample16() {}
    virtual ~AudioSample16() override;

    virtual bool Open(PAudioDataSource data_source) override;
    virtual bool IsValid() override;
    virtual bool IsStopped() override;

    virtual bool Play(bool loop = false, bool positioned = false) override;
    virtual bool Stop() override;
    virtual bool Pause() override;
    virtual bool Resume() override;
    virtual bool SetVolume(float volume) override;
    virtual bool SetPosition(float x, float y, float z, float max_dist) override;

 protected:
    void Close();
    void defaultSource();

    PAudioDataSource pDataSource = nullptr;
    ALuint al_source = -1;
    Vec3f _position = Vec3f(0.0, 0.0, 0.0);
    float _maxDistance = 0.0;
    float _volume = 0.0;
};

AudioSample16::~AudioSample16() { Close(); }

void AudioSample16::Close() {
    pDataSource = nullptr;

    if (alIsSource(al_source) != 0) {
        alSourceStop(al_source);
        CheckError();
        alSourcei(al_source, AL_BUFFER, 0);
        CheckError();
        alDeleteSources(1, &al_source);
        CheckError();
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

    alGenSources((ALuint)1, &al_source);
    if (CheckError()) {
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
        if (CheckError()) {
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
    if (CheckError()) {
        return false;
    }

    return true;
}

bool AudioSample16::Stop() {
    if (!IsValid()) {
        return false;
    }

    alSourceStop(al_source);
    if (CheckError()) {
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
    if (CheckError()) {
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
    if (CheckError()) {
        return false;
    }

    return true;
}

bool AudioSample16::SetVolume(float volume) {
    _volume = volume;

    if (IsValid()) {
        alSourcef(al_source, AL_GAIN, volume);
        if (CheckError()) {
            return false;
        }
    }

    return true;
}

PAudioSample CreateAudioSample() {
    return std::make_shared<AudioSample16>();
}
