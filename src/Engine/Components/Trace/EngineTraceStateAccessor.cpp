#include "EngineTraceStateAccessor.h"

#include <string>

#include "Application/GameConfig.h"

#include "Engine/Party.h"
#include "Engine/mm7_data.h"

#include "Library/Trace/EventTrace.h"

#include "Utility/Exception.h"
#include "Utility/String.h"

static bool shouldSkip(const GameConfig *config, const ConfigSection *section, const AnyConfigEntry *entry) {
    return
        (section == &config->window && entry != &config->window.Width && entry != &config->window.Height) ||
        section == &config->graphics ||
        entry == &config->settings.MusicLevel ||
        entry == &config->settings.VoiceLevel ||
        entry == &config->settings.SoundLevel ||
        entry == &config->debug.LogLevel ||
        entry == &config->debug.NoVideo;
}

static bool shouldTake(const GameConfig *config, const ConfigSection *section, const AnyConfigEntry *entry) {
    return
        entry->string() != entry->defaultString() ||
        entry == &config->debug.TraceFrameTimeMs ||
        entry == &config->debug.TraceRandomEngine;
}

void EngineTraceStateAccessor::prepareForPlayback(GameConfig *config) {
    config->settings.MusicLevel.setValue(0);
    config->settings.VoiceLevel.setValue(0);
    config->settings.SoundLevel.setValue(0); // Note: still need to call AudioPlayer::UpdateVolumeFromConfig.
    config->window.MouseGrab.setValue(false);
    config->graphics.FPSLimit.setValue(0); // Unlimited
    config->debug.NoVideo.setValue(true);
}

std::vector<EventTraceConfigLine> EngineTraceStateAccessor::makeConfigPatch(const GameConfig *config) {
    std::vector<EventTraceConfigLine> result;
    for (const ConfigSection *section : config->sections())
        for (const AnyConfigEntry *entry : section->entries())
            if (!shouldSkip(config, section, entry) && shouldTake(config, section, entry))
                result.push_back({section->name(), entry->name(), entry->string()});
    return result;
}

void EngineTraceStateAccessor::patchConfig(GameConfig *config, const std::vector<EventTraceConfigLine>& patch) {
    for (ConfigSection *section : config->sections())
        for (AnyConfigEntry *entry : section->entries())
            if (!shouldSkip(config, section, entry))
                entry->reset();

    // TODO(captainurist): Right now setting keybindings here doesn't work
    for (const EventTraceConfigLine &line : patch) {
        ConfigSection *section = config->section(line.section);
        if (!section)
            throw Exception("Config section '{}' doesn't exist", line.section);

        AnyConfigEntry *entry = section->entry(line.key);
        if (!entry)
            throw Exception("Config entry '{}.{}' doesn't exist", line.section, line.key);

        entry->setString(line.value);
    }
}

EventTraceGameState EngineTraceStateAccessor::makeGameState() {
    EventTraceGameState result;
    result.locationName = toLower(pCurrentMapName);
    result.partyPosition = pParty->pos.toInt();
    return result;
}
