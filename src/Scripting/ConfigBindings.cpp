#include "ConfigBindings.h"

#include <functional>
#include <string>
#include <ranges>
#include <vector>

#include "Engine/Engine.h"

#include "Utility/Exception.h"
#include "Utility/SmallVector.h"

sol::table ConfigBindings::createBindingTable(sol::state_view &solState) const {
    return solState.create_table_with(
        "setConfig", sol::overload(setConfigValue1, setConfigValue2),
        "getConfig", sol::overload(configValue1, configValue2),
        "resetConfig", sol::overload(resetConfigValue1, resetConfigValue2),
        "listConfigs", sol::as_function([&solState](const char *section, const char *filter) {
                return listConfigValues(solState, section, filter);
            })
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

std::optional<std::string> ConfigBindings::resetConfigValue1(std::string_view sectionName, std::string_view entryName) {
    ConfigSection *section = engine->config->section(sectionName);
    if (!section)
        return std::nullopt;

    AnyConfigEntry *configEntry = section->entry(entryName);
    if (!configEntry)
        return std::nullopt;

    configEntry->setValue(configEntry->defaultValue());
    return configEntry->defaultString();
}

std::optional<std::string> ConfigBindings::resetConfigValue2(std::string_view entryName) {
    for (auto &&section : engine->config->sections()) {
        AnyConfigEntry *configEntry = section->entry(entryName);
        if (configEntry != nullptr) {
            configEntry->setValue(configEntry->defaultValue());
            return configEntry->defaultString();
        }
    }

    return std::nullopt;
}

sol::table ConfigBindings::listConfigValues(sol::state_view &solState, std::string_view sectionName, std::string_view filter) {
    sol::table result = solState.create_table();
    for (AnyConfigEntry *entry : findConfigEntries(sectionName, filter)) {
        result.add(solState.create_table_with(
                "section", entry->section()->name(),
                "name", entry->name(),
                "type", configEntryTypeName(entry),
                "description", entry->description(),
                "value", entry->string(),
                "default", entry->defaultString()
        ));
    }
    return result;
}

std::vector<AnyConfigEntry *> ConfigBindings::findConfigEntries(std::string_view sectionName, std::string_view filter) {
    std::vector<AnyConfigEntry *> result;
    ConfigSection *sectionFilter = nullptr;
    std::string valueFilter;
    bool defaultFilter, filterByDefault = false, filterByName = false;
    std::string name(filter.empty() ? sectionName : filter);

    if (!sectionName.empty()) {
        for (ConfigSection *section : engine->config->sections()) {
            if (section->name() == sectionName) {
                sectionFilter = section;
                if (filter.empty()) name = "";  // sectionFilter was actually the section name, so no name/value filtering within sectionFilter
                break;
            }
        }
    }

    if (name.empty()) {
        // Ensure numeric test below is skipped
    } else if (name == "default") {
        defaultFilter = true;
        filterByDefault = true;
    } else if (name == "!default") {
        defaultFilter = false;
        filterByDefault = true;
    } else if (name == "false" || name == "true") {
        valueFilter = name;
    } else if (name.starts_with('"') && name.ends_with('"')) {
        valueFilter = name.substr(1, name.size() - 2);
    } else if (name.find_first_not_of("0123456789") == std::string::npos) {
        valueFilter = name;
    } else {
        filterByName = true;
    }

    for (ConfigSection *section : engine->config->sections()) {
        if (sectionFilter && section != sectionFilter)
            continue;
        for (AnyConfigEntry *entry : section->entries()) {
            if (filterByDefault && (entry->string() == entry->defaultString()) != defaultFilter)
                continue;
            if (filterByName && entry->name().find(name) == std::string::npos)
                continue;
            if (!valueFilter.empty() && entry->string() != valueFilter)
                continue;
            result.emplace_back(entry);
        }
    }
    return result;
}

std::string ConfigBindings::configEntryTypeName(AnyConfigEntry *entry) {
    if (entry->type() == typeid(bool)) return "boolean";
    if (entry->type() == typeid(int)) return "integer";
    if (entry->type() == typeid(float)) return "float";
    if (entry->type() == typeid(std::string)) return "string";
    if (entry->type() == typeid(PlatformKey)) return "key";
    return "unknown";
}
