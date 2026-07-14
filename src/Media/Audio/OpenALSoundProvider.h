#pragma once

#include <cstddef>

#include <al.h> // NOLINT: not a C system header.
#include <alc.h> // NOLINT: not a C system header.

// Sound attenuation factors
constexpr float MAX_SOUND_DIST = 60000.0f;
constexpr float REFERENCE_DIST = 300.0f;
constexpr float ROLLOFF_FACTOR = 1.5f;

class OpenALSoundProvider {
 public:
    struct StreamingTrackBuffer {
        unsigned int source_id = 0;
        ALenum sample_format = 0;
        int sample_rate = 0;
    };

    OpenALSoundProvider();
    virtual ~OpenALSoundProvider();

    bool Initialize();
    void Release();

    void DeleteStreamingTrack(StreamingTrackBuffer **buffer);
    StreamingTrackBuffer *CreateStreamingTrack16(int num_channels,
                                                 int sample_rate,
                                                 int bytes_per_sample);
    void Stream16(StreamingTrackBuffer *buffer, int num_samples,
                  const void *samples, bool wait = false);
    void SetListenerPosition(float x, float y, float z);
    void SetOrientation(float yaw, float pitch);

 protected:
    void DeleteBuffers(StreamingTrackBuffer *track, int type);

    ALCdevice *device = nullptr;
    ALCcontext *context = nullptr;
};

// TODO(pskelton): contain?
void setSourceDefaults(ALuint al_source);
bool checkOpenALError();
