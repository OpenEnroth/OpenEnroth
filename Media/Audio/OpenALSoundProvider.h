#pragma once
#include "lib/OpenAL/al.h"
#include "lib/OpenAL/alc.h"
#pragma comment(lib, "OpenAL32.lib")

#include "Engine/Engine.h"

void log(char *format, ...)
{
    va_list va;
    va_start(va, format);
    char msg[256];
    vsprintf(msg, format, va);
    va_end(va);
    DWORD w;

    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), msg, strlen(msg), &w, 0);
}

class OpenALSoundProvider
{
  public:
    struct TrackBuffer
    {
      unsigned int source_id;
      unsigned int buffer_id;
    };

    struct StreamingTrackBuffer
    {
      unsigned int source_id;
      ALenum       sample_format;
      int          sample_rate;
    };

    inline OpenALSoundProvider()
    {
      this->device = nullptr;
      this->context = nullptr;
    }

    inline ~OpenALSoundProvider()
    {
      Release();
    }

    inline bool Initialize()
    {

      auto device_names = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
      if (!device_names)
      {
        device_names = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
      }
      if (device_names)
      {
        for (auto device_name = device_names; device_name[0]; device_name += strlen(device_name))
        {
          continue;
        }
      }

      device = alcOpenDevice(nullptr);
      if (!device || CheckError())
	  {
        Log::Warning(L"Default sound device not present");
        return false;
	  }

      context = alcCreateContext(device, nullptr);
      if (!context || CheckError())
        return Release(), false;

      alcMakeContextCurrent(context);

      bool eax2 = alIsExtensionPresent("EAX2.0");
      bool eax3 = alIsExtensionPresent("EAX3.0");
      bool eax4 = alIsExtensionPresent("EAX4.0");
      bool eax5 = alIsExtensionPresent("EAX5.0");
      
      auto vendor = alGetString(AL_VENDOR);
      auto version = alGetString(AL_VERSION);
      auto extensions = alcGetString(device, ALC_EXTENSIONS);

      return true;
    }

    void Release()
    {
      alcMakeContextCurrent(nullptr);
      if (context)
      {
        alcDestroyContext(context);
      }
      if (device)
      {
        alcCloseDevice(device);
      }
    }

    void DeleteStreamingTrack(StreamingTrackBuffer **buffer)
    {
      if (!buffer && !*buffer)
        return;
      auto track = *buffer;

      int status;
      alGetSourcei(track->source_id, AL_SOURCE_STATE, &status);
      if (status == AL_PLAYING)
      {
        alSourceStop(track->source_id);
        if (CheckError()) __debugbreak();
      }

      int num_processed_buffers = 0;
      int num_queued_buffers = 0;
      alGetSourcei(track->source_id, AL_BUFFERS_PROCESSED, &num_processed_buffers);
      alGetSourcei(track->source_id, AL_BUFFERS_QUEUED, &num_queued_buffers);
      int num_track_buffers = num_queued_buffers + num_processed_buffers;
      for (int i = 0; i < num_processed_buffers; ++i)
      {
        unsigned int buffer_id;
        alSourceUnqueueBuffers(track->source_id, 1, &buffer_id);
        if (!CheckError())
          alDeleteBuffers(1, &buffer_id);
        else __debugbreak();
      }

      alDeleteSources(1, &track->source_id);
      CheckError();

      delete *buffer;
      *buffer = nullptr;
    }

    void DeleteBuffer16(TrackBuffer **buffer)
    {
      alDeleteBuffers(1, &(*buffer)->buffer_id);
      CheckError();

      delete *buffer;
      *buffer = nullptr;
    }

    float alBufferLength(unsigned int buffer)
    {
      int size, bits, channels, freq;

      alGetBufferi(buffer, AL_SIZE, &size);
      alGetBufferi(buffer, AL_BITS, &bits);
      alGetBufferi(buffer, AL_CHANNELS, &channels);
      alGetBufferi(buffer, AL_FREQUENCY, &freq);
      if (CheckError())
        return 0.0f;

      return (ALfloat)((ALuint)size / channels / (bits / 8)) / (ALfloat)freq;
    }

    StreamingTrackBuffer *CreateStreamingTrack16(int num_channels, int sample_rate, int bytes_per_sample)
    {
      Assert(bytes_per_sample == 2, "OpenALSoundProvider: unsupported sample size: %u", bytes_per_sample);

      ALenum sound_format;
      switch (num_channels)
      {
        case 1: sound_format = AL_FORMAT_MONO16;    break;
        case 2: sound_format = AL_FORMAT_STEREO16;  break;
        default:
          if (bool multichannel = alIsExtensionPresent("AL_EXT_MCFORMATS"))
          {
            switch (num_channels)
            {
              case 4: sound_format = alGetEnumValue("AL_FORMAT_QUAD16");  break;
              case 6: sound_format = alGetEnumValue("AL_FORMAT_51CHN16"); break;
              case 7: sound_format = alGetEnumValue("AL_FORMAT_61CHN16"); break;
              case 8: sound_format = alGetEnumValue("AL_FORMAT_71CHN16"); break;
            }
          }
          Error("Unsupported number of audio channels: %u", num_channels);
      }

      unsigned int al_source = -1;
      alGetError();
      alGenSources(1, &al_source);
      if (CheckError())
        return nullptr;

      float sound_pos[] = {0.0f, 0.0f, 0.0f},
            sound_vel[] = {0.0f, 0.0f, 0.0f};

      alSourcei(al_source, AL_LOOPING, AL_FALSE);
      alSourcef(al_source, AL_PITCH, 1.0f);
      alSourcef(al_source, AL_GAIN, 1.0f);
      alSourcefv(al_source, AL_POSITION, sound_pos);
      alSourcefv(al_source, AL_VELOCITY, sound_vel);

      auto ret = new StreamingTrackBuffer;
      ret->source_id = al_source;
      ret->sample_format = sound_format;
      ret->sample_rate = sample_rate;
      return ret;
    }

    void Stream16(StreamingTrackBuffer *buffer, int num_samples, const void *samples, bool wait = false)
    {
      int bytes_per_sample = 2;

      unsigned int al_buffer;
      alGenBuffers(1, &al_buffer);
      alBufferData(al_buffer, buffer->sample_format, samples, num_samples * bytes_per_sample, buffer->sample_rate);
      if (CheckError())
      {
        alDeleteBuffers(1, &al_buffer);
        return;
      }

      int num_processed_buffers = 0;
      alGetSourcei(buffer->source_id, AL_BUFFERS_PROCESSED, &num_processed_buffers);
      for (int i = 0; i < num_processed_buffers; ++i)
      {
        unsigned int processed_buffer_id;
        alSourceUnqueueBuffers(buffer->source_id, 1, &processed_buffer_id);
        if (!CheckError())
          alDeleteBuffers(1, &processed_buffer_id);
      }

      alSourceQueueBuffers(buffer->source_id, 1, &al_buffer);
      if (CheckError())
      {
        alDeleteBuffers(1, &al_buffer);
        return;
      }

      volatile int status;
      alGetSourcei(buffer->source_id, AL_SOURCE_STATE, (int *)&status);
      if (status != AL_PLAYING)
      {
        float listener_pos[] = {0.0f, 0.0f, 0.0f};
        float listener_vel[] = {0.0f, 0.0f, 0.0f};
        float listener_orientation[] = {0.0f, 0.0f, -1.0f, // direction
                                        0.0f, 1.0f, 0.0f}; // up vector
        alListenerfv(AL_POSITION, listener_pos);
        alListenerfv(AL_VELOCITY, listener_vel);
        alListenerfv(AL_ORIENTATION, listener_orientation);

        alSourcePlay(buffer->source_id);
        if (CheckError())
          __debugbreak();

        if (wait)
        {
          do
          {
            alGetSourcei(buffer->source_id, AL_SOURCE_STATE, (int *)&status);
          }
          while (status == AL_PLAYING);
        }
      }
    }




    TrackBuffer *CreateTrack16(int num_channels, int sample_rate, int bytes_per_sample, int num_samples, const void *samples)
    {
      Assert(bytes_per_sample == 2, "OpenALSoundProvider: unsupported sample size: %u", bytes_per_sample);

      ALenum sound_format;
      switch (num_channels)
      {
        case 1: sound_format = AL_FORMAT_MONO16;    break;
        case 2: sound_format = AL_FORMAT_STEREO16;  break;
        default:
          if (bool multichannel = alIsExtensionPresent("AL_EXT_MCFORMATS"))
          {
            switch (num_channels)
            {
              case 4: sound_format = alGetEnumValue("AL_FORMAT_QUAD16");  break;
              case 6: sound_format = alGetEnumValue("AL_FORMAT_51CHN16"); break;
              case 7: sound_format = alGetEnumValue("AL_FORMAT_61CHN16"); break;
              case 8: sound_format = alGetEnumValue("AL_FORMAT_71CHN16"); break;
            }
          }
          Error("Unsupported number of audio channels: %u", num_channels);
      }

      unsigned int al_source = -1;
      alGenSources(1, &al_source);
      if (CheckError())
        return nullptr;

      float sound_pos[] = {0.0f, 0.0f, 0.0f},
            sound_vel[] = {0.0f, 0.0f, 0.0f};

      alSourcei(al_source, AL_LOOPING, AL_FALSE);
      alSourcef(al_source, AL_PITCH, 1.0f);
      alSourcef(al_source, AL_GAIN, 1.0f);
      alSourcefv(al_source, AL_POSITION, sound_pos);
      alSourcefv(al_source, AL_VELOCITY, sound_vel);

      unsigned int al_buffer = -1;
      alGenBuffers(1, &al_buffer);
      if (CheckError())
      {
        alDeleteSources(1, &al_source);
        return nullptr;
      }

      alBufferData(al_buffer, sound_format, samples, num_samples * bytes_per_sample, sample_rate);
      if (CheckError())
      {
        alDeleteSources(1, &al_source);
        alDeleteBuffers(1, &al_buffer);
        return nullptr;
      }

      alSourcei(al_source, AL_BUFFER, al_buffer);
      if (CheckError())
      {
        alDeleteSources(1, &al_source);
        alDeleteBuffers(1, &al_buffer);
        return nullptr;
      }

      auto ret = new TrackBuffer;
      ret->source_id = al_source;
      ret->buffer_id = al_buffer;
      return ret;
    }


    void PlayTrack16(TrackBuffer *buffer, bool loop = false, bool wait = false)
    {
      volatile int status;
      alGetSourcei(buffer->source_id, AL_SOURCE_STATE, (int *)&status);
      if (status == AL_PLAYING)
        Error("Already playing");
      else
      {
        float listener_pos[] = {0.0f, 0.0f, 0.0f};
        float listener_vel[] = {0.0f, 0.0f, 0.0f};
        float listener_orientation[] = {0.0f, 0.0f, -1.0f, // direction
                                        0.0f, 1.0f, 0.0f}; // up vector
        alListenerfv(AL_POSITION, listener_pos);
        alListenerfv(AL_VELOCITY, listener_vel);
        alListenerfv(AL_ORIENTATION, listener_orientation);

        alSourcei(buffer->source_id, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        alSourcePlay(buffer->source_id);
        if (CheckError())
          __debugbreak();

        if (wait && !loop)
        {
          float track_length = alBufferLength(buffer->buffer_id);
          do
          {
            float track_offset = 0;
            alGetSourcef(buffer->source_id, AL_SEC_OFFSET, &track_offset);
            log("playing: %.4f/%.4f\n", track_offset, track_length);

            alGetSourcei(buffer->source_id, AL_SOURCE_STATE, (int *)&status);
          }
          while (status == AL_PLAYING);
        }
      }
    }



  protected:
    ALCdevice    *device;
    ALCcontext   *context;


    bool CheckError()
    {
      ALenum code1 = alGetError();
      if (code1 != AL_NO_ERROR)
      {
        DWORD w;
        const char *message = alGetString(code1);
        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, lstrlenA(message), &w, nullptr);
        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), "\n", 1, &w, nullptr);
        return true;
      }

      ALenum code2 = alcGetError(device);
      if (code2 != ALC_NO_ERROR)
      {
        DWORD w;
        const char *message = alcGetString(device, code2);
        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, lstrlenA(message), &w, nullptr);
        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), "\n", 1, &w, nullptr);
        return true;
      }
      return false;
    }
};