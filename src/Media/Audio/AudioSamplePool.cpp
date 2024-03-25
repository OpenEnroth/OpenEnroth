#include "AudioSamplePool.h"

SoundPlaybackResult AudioSamplePool::playNew(PAudioSample sample, PAudioDataSource source, bool positional) {
    update();
    if (!sample->Open(source)) {
        return SOUND_PLAYBACK_FAILED;
    }
    sample->Play(_looping, positional);
    _samplePool.push_back(AudioSamplePoolEntry(sample, SOUND_Invalid, Pid()));
    return SOUND_PLAYBACK_SUCCEEDED;
}

SoundPlaybackResult AudioSamplePool::playUniqueSoundId(PAudioSample sample, PAudioDataSource source, SoundId id, bool positional) {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        if (entry.id == id) {
            return SOUND_PLAYBACK_SKIPPED;
        }
    }
    if (!sample->Open(source)) {
        return SOUND_PLAYBACK_FAILED;
    }
    sample->Play(_looping, positional);
    _samplePool.push_back(AudioSamplePoolEntry(sample, id, Pid()));
    return SOUND_PLAYBACK_SUCCEEDED;
}

SoundPlaybackResult AudioSamplePool::playUniquePid(PAudioSample sample, PAudioDataSource source, Pid pid, bool positional) {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        if (entry.pid == pid) {
            return SOUND_PLAYBACK_SKIPPED;
        }
    }
    if (!sample->Open(source)) {
        return SOUND_PLAYBACK_FAILED;
    }
    sample->Play(_looping, positional);
    _samplePool.push_back(AudioSamplePoolEntry(sample, SOUND_Invalid, pid));
    return SOUND_PLAYBACK_SUCCEEDED;
}

void AudioSamplePool::pause() {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->Pause();
    }
}

void AudioSamplePool::resume() {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->Resume();
    }
}

void AudioSamplePool::stop() {
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->Stop();
    }
    _samplePool.clear();
}

void AudioSamplePool::stopSoundId(SoundId soundId) {
    assert(soundId != SOUND_Invalid);

    auto it = _samplePool.begin();
    while (it != _samplePool.end()) {
        if ((*it).id == soundId) {
            (*it).samplePtr->Stop();
            it = _samplePool.erase(it);
        } else {
            it++;
        }
    }
}

void AudioSamplePool::stopPid(Pid pid) {
    assert(pid != Pid());

    auto it = _samplePool.begin();
    while (it != _samplePool.end()) {
        if ((*it).pid == pid) {
            (*it).samplePtr->Stop();
            it = _samplePool.erase(it);
        } else {
            it++;
        }
    }
}

void AudioSamplePool::update() {
    auto it = _samplePool.begin();
    std::erase_if(_samplePool, [](const AudioSamplePoolEntry& entry) { return entry.samplePtr->IsStopped(); });
}

void AudioSamplePool::setVolume(float value) {
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->SetVolume(value);
    }
}

bool AudioSamplePool::hasPlaying() {
    for (AudioSamplePoolEntry &entry : _samplePool) {
        if (!entry.samplePtr->IsStopped()) {
            return true;
        }
    }
    return false;
}
