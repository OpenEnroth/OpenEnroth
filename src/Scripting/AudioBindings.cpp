#include "AudioBindings.h"

#include <Media/Audio/AudioPlayer.h>

sol::table AudioBindings::createBindingTable(sol::state_view &solState) const {
    return solState.create_table_with(
        "playSound", sol::as_function([](SoundId soundId, SoundPlaybackMode mode) {
            pAudioPlayer->playSound(soundId, mode);
        }),
        "playMusic", sol::as_function([](MusicId musicId) {
            pAudioPlayer->MusicPlayTrack(musicId);
        })
    );
}
