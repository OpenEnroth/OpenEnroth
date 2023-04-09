#include "EngineTraceConfigurator.h"

#include <string>

#include "Application/GameConfig.h"

#include "Library/Trace/EventTrace.h"

#include "Utility/Exception.h"

static bool shouldSkip(const GameConfig *config, const ConfigSection *section) {
    return section == &config->window || section == &config->graphics;
}

static bool shouldSkip(const GameConfig *config, const AnyConfigEntry *entry) {
    return entry == &config->debug.VerboseLogging;
}

std::vector<EventTraceConfigLine> EngineTraceConfigurator::makeConfigPatch(const GameConfig *config) {
    std::vector<EventTraceConfigLine> result;
    for (const ConfigSection *section : config->sections())
        if (!shouldSkip(config, section))
            for (const AnyConfigEntry *entry : section->entries())
                if (!shouldSkip(config, entry) && entry->string() != entry->defaultString())
                    result.push_back({section->name(), entry->name(), entry->string()});
    return result;
}

void EngineTraceConfigurator::patchConfig(GameConfig *config, const std::vector<EventTraceConfigLine>& patch) {
    for (ConfigSection *section : config->sections())
        if (!shouldSkip(config, section))
            for (AnyConfigEntry *entry : section->entries())
                if (!shouldSkip(config, entry))
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
