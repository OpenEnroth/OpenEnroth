#include "EngineTraceStateAccessor.h"

#include <string>

#include "Application/GameConfig.h"

#include "Engine/Party.h"
#include "Engine/Engine.h"
#include "Engine/MapInfo.h"
#include "Engine/mm7_data.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Trace/EventTrace.h"

#include "Utility/String/Ascii.h"

static bool shouldSkip(const GameConfig *config, const ConfigSection *section, const AnyConfigEntry *entry) {
    return
        (section == &config->window && entry != &config->window.Width && entry != &config->window.Height) ||
        section == &config->graphics ||
        entry == &config->settings.MusicLevel ||
        entry == &config->settings.VoiceLevel ||
        entry == &config->settings.SoundLevel ||
        entry == &config->debug.LogLevel ||
        entry == &config->debug.NoVideo ||
        entry == &config->gameplay.NoPartyActorCollisions ||
        entry == &config->gameplay.QuickSavesCount;
}

static bool shouldTake(const GameConfig *config, const ConfigSection *section, const AnyConfigEntry *entry) {
    return
        entry->string() != entry->defaultString() ||
        entry == &config->debug.TraceFrameTimeMs ||
        entry == &config->debug.TraceRandomEngine ||
        entry == &config->debug.TraceNoVideo ||
        entry == &config->debug.TraceNoPartyActorCollisions;
}

void EngineTraceStateAccessor::prepareForRecording(GameConfig *config, ConfigPatch *patch) {
    *patch = ConfigPatch::fromConfig(config, [config] (const ConfigSection *section, const AnyConfigEntry *entry) {
        return !shouldSkip(config, section, entry) && shouldTake(config, section, entry);
    });

    config->graphics.FPSLimit.setValue(1000 / config->debug.TraceFrameTimeMs.value());
    config->debug.NoVideo.setValue(config->debug.TraceNoVideo.value());
    config->gameplay.NoPartyActorCollisions.setValue(config->debug.TraceNoPartyActorCollisions.value());
}

void EngineTraceStateAccessor::prepareForPlayback(GameConfig *config, const ConfigPatch &patch) {
    for (ConfigSection *section : config->sections())
        for (AnyConfigEntry *entry : section->entries())
            if (!shouldSkip(config, section, entry))
                entry->reset();

    // TODO(captainurist): Right now setting keybindings here doesn't work
    patch.apply(config);

    config->settings.MusicLevel.setValue(0);
    config->settings.VoiceLevel.setValue(0);
    config->settings.SoundLevel.setValue(0);
    config->window.MouseGrab.setValue(false);
    config->graphics.FPSLimit.setValue(0); // Unlimited
    config->debug.NoVideo.setValue(config->debug.TraceNoVideo.value());
    config->gameplay.NoPartyActorCollisions.setValue(config->debug.TraceNoPartyActorCollisions.value());
    pAudioPlayer->UpdateVolumeFromConfig();
}

EventTraceGameState EngineTraceStateAccessor::makeGameState() {
    EventTraceGameState result;
    result.locationName = ascii::toLower(pMapStats->pInfos[engine->_currentLoadedMapId].fileName);
    result.partyPosition = pParty->pos.toInt();
    for (const Character &character : pParty->pCharacters) {
        EventTraceCharacterState &traceCharacter = result.characters.emplace_back();
        traceCharacter.hp = character.health;
        traceCharacter.mp = character.mana;
        traceCharacter.might = character.GetActualMight();
        traceCharacter.intelligence = character.GetActualIntelligence();
        traceCharacter.personality = character.GetActualPersonality();
        traceCharacter.endurance = character.GetActualEndurance();
        traceCharacter.accuracy = character.GetActualAccuracy();
        traceCharacter.speed = character.GetActualSpeed();
        traceCharacter.luck = character.GetActualLuck();
    }
    return result;
}
