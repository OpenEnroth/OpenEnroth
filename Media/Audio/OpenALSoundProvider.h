#pragma once

#ifdef __APPLE__
#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <memory>
#include <string>

#include "Media/Media.h"

void log(const char *format, ...);

class OpenALSoundProvider {
 public:
    struct TrackBuffer {
        unsigned int source_id;
        unsigned int buffer_id;
    };

    struct StreamingTrackBuffer {
        unsigned int source_id;
        ALenum sample_format;
        int sample_rate;
    };

    OpenALSoundProvider();
    virtual ~OpenALSoundProvider();

    bool Initialize();
    void Release();

    void DeleteStreamingTrack(StreamingTrackBuffer **buffer);
    void DeleteBuffer16(TrackBuffer **buffer);
    float alBufferLength(unsigned int buffer);
    StreamingTrackBuffer *CreateStreamingTrack16(int num_channels,
                                                 int sample_rate,
                                                 int bytes_per_sample);
    void Stream16(StreamingTrackBuffer *buffer, int num_samples,
                  const void *samples, bool wait = false);
    TrackBuffer *CreateTrack16(int num_channels, int sample_rate,
                               const void *data, size_t size);
    void PlayTrack16(TrackBuffer *buffer, bool loop = false, bool wait = false);
    void SetListenerPosition(float x, float y, float z);
    void SetOrientation(float yaw, float pitch);

 protected:
    ALCdevice *device;
    ALCcontext *context;
};
