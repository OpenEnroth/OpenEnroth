#pragma once

#include <list>

#include "Engine/Pid.h"

#include "Media/AudioSample.h"

#include "SoundEnums.h"

struct AudioSamplePoolEntry {
    AudioSamplePoolEntry(PAudioSample samplePtr, SoundId id, Pid pid) : samplePtr(samplePtr), id(id), pid(pid) {}

    PAudioSample samplePtr;
    SoundId id;
    Pid pid;
};

class AudioSamplePool {
 public:
    explicit AudioSamplePool(bool looping):_looping(looping) {}

    SoundPlaybackResult playNew(PAudioSample sample, PAudioDataSource source, bool positional = false);
    SoundPlaybackResult playUniqueSoundId(PAudioSample sample, PAudioDataSource source, SoundId id, bool positional = false);
    SoundPlaybackResult playUniquePid(PAudioSample sample, PAudioDataSource source, Pid pid, bool positional = false);
    void pause();
    void resume();
    void stop();
    void stopSoundId(SoundId soundId);
    void stopPid(Pid pid);
    void update();
    void setVolume(float value);
    bool hasPlaying();
 private:
    std::list<AudioSamplePoolEntry> _samplePool;
    bool _looping;
};
