#include "ConfigBindings.h"

#include <functional>
#include <string>
#include <ranges>

#include "Engine/Engine.h"

#include "Utility/Exception.h"
#include "Utility/SmallVector.h"

sol::table ConfigBindings::createBindingTable(sol::state_view &solState) const {
    return solState.create_table_with(
        "setConfig", sol::overload(setConfigValue1, setConfigValue2),
        "getConfig", sol::overload(configValue1, configValue2)
    );
}

void ConfigBindings::setConfigValue1(std::string_view sectionName, std::string_view entryName, std::string_view value) {
    locateEntry1(sectionName, entryName)->setString(value);
}

void ConfigBindings::setConfigValue2(std::string_view entryName, std::string_view value) {
    locateEntry2(entryName)->setString(value);
}

std::string ConfigBindings::configValue1(std::string_view sectionName, std::string_view entryName) {
    return locateEntry1(sectionName, entryName)->string();
}

std::string ConfigBindings::configValue2(std::string_view entryName) {
    return locateEntry2(entryName)->string();
}

AnyConfigEntry *ConfigBindings::locateEntry1(std::string_view sectionName, std::string_view entryName) {
    ConfigSection *section = engine->config->section(sectionName);
    if (!section)
        throw Exception("Can't find config section '{}'", sectionName);

    AnyConfigEntry *entry = section->entry(entryName);
    if (!entry)
        throw Exception("Can't find config entry '{}.{}'", sectionName, entryName);

    return entry;
}

AnyConfigEntry *ConfigBindings::locateEntry2(std::string_view entryName) {
    gch::small_vector<AnyConfigEntry *, 16> entries;

    for (const ConfigSection *section : engine->config->sections())
        if (AnyConfigEntry *entry = section->entry(entryName))
            entries.push_back(entry);

    if (entries.empty())
        throw Exception("Can't find config entry '{}'", entryName);

    if (entries.size() > 1) {
        auto entryStrings = entries | std::views::transform([] (AnyConfigEntry *entry) {
            return fmt::format("'{}.{}'", entry->section()->name(), entry->name());
        });
        throw Exception("Several config entries named '{}' found ({})", entryName, fmt::join(entryStrings, ", "));
    }

    return entries[0];
}
