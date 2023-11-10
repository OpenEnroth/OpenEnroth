#pragma once

#include <list>

#include "Engine/Pid.h"

#include "Media/AudioSample.h"

#include "SoundEnums.h"

struct AudioSamplePoolEntry {
    AudioSamplePoolEntry(PAudioSample samplePtr, SoundID id, Pid pid) : samplePtr(samplePtr), id(id), pid(pid) {}

    PAudioSample samplePtr;
    SoundID id;
    Pid pid;
};

class AudioSamplePool {
 public:
    explicit AudioSamplePool(bool looping):_looping(looping) {}

    bool playNew(PAudioSample sample, PAudioDataSource source, bool positional = false);
    bool playUniqueSoundId(PAudioSample sample, PAudioDataSource source, SoundID id, bool positional = false);
    bool playUniquePid(PAudioSample sample, PAudioDataSource source, Pid pid, bool positional = false);
    void pause();
    void resume();
    void stop();
    void stopSoundId(SoundID soundId);
    void stopPid(Pid pid);
    void update();
    void setVolume(float value);
    bool hasPlaying();
 private:
    std::list<AudioSamplePoolEntry> _samplePool;
    bool _looping;
};
