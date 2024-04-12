#include "Config.h"

#include <cassert>
#include <filesystem>
#include <string>
#include <vector>

#define MINI_CASE_SENSITIVE
#include <mini/ini.h> // NOLINT: this is not a C system header.

#include "Utility/MapAccess.h"
#include "Utility/Exception.h"

void Config::load(std::string_view path) {
    if (!std::filesystem::exists(path))
        throw Exception("config file '{}' doesn't exist", path);

    mINI::INIFile file = mINI::INIFile(std::string(path));
    mINI::INIStructure ini;
    if (!file.read(ini))
        throw Exception("Couldn't read config file '{}'", path);

    for (const auto &[sectionName, iniSection] : ini)
        if (ConfigSection *section = this->section(sectionName))
            for (const auto &[entryName, iniValue] : iniSection)
                if (AnyConfigEntry *entry = section->entry(entryName))
                    entry->setString(iniValue);
}

void Config::save(std::string_view path) const {
    mINI::INIFile file = mINI::INIFile(std::string(path));
    mINI::INIStructure ini;

    for (ConfigSection *section : sections())
        for (AnyConfigEntry *entry : section->entries())
            ini[section->name()][entry->name()] = entry->string();

    if (!file.write(ini, true))
        throw Exception("Couldn't save config file '{}'", path);
}

void Config::reset() {
    for (ConfigSection *section : sections())
        for (AnyConfigEntry *entry : section->entries())
            entry->reset();
}

void Config::registerSection(ConfigSection *section) {
    assert(section);
    assert(!_sectionByName.contains(section->name()));

    _sectionByName.emplace(section->name(), section);
}

ConfigSection *Config::section(std::string_view name) const {
    return valueOr(_sectionByName, name, nullptr);
}

std::vector<ConfigSection *> Config::sections() const {
    std::vector<ConfigSection *> result;
    for (const auto &[_, section] : _sectionByName)
        result.push_back(section);
    return result;
}
