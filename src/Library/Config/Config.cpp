#include "Config.h"

#define MINI_CASE_SENSITIVE
#include <mini/ini.h>

#include <cassert>
#include <stdexcept>
#include <filesystem>
#include <string>

#include "Utility/MapAccess.h"

void Config::Load(const std::string &path) {
    if (!std::filesystem::exists(path))
        throw std::runtime_error("Config file doesn't exist"); // TODO(captainurist): exception formatting

    mINI::INIFile file(path);
    mINI::INIStructure ini;
    if (!file.read(ini))
        throw std::runtime_error("Couldn't read config file"); // TODO(captainurist): exception formatting

    for (const auto &[sectionName, iniSection] : ini)
        if (ConfigSection *section = Section(sectionName))
            for (const auto &[valueName, valueString] : iniSection)
                if (AbstractConfigValue *value = section->Value(valueName))
                    value->SetString(valueString);
}

void Config::Save(const std::string &path) const {
    mINI::INIFile file(path);
    mINI::INIStructure ini;

    for (ConfigSection *section : Sections())
        for (AbstractConfigValue *value : section->Values())
            ini[section->Name()][value->Name()] = value->GetString();

    if (!file.write(ini, true))
        throw std::runtime_error("Couldn't save config file"); // TODO(captainurist): exception formatting
}

void Config::Reset() {
    for (ConfigSection *section : Sections())
        for (AbstractConfigValue *value : section->Values())
            value->Reset();
}

void Config::RegisterSection(ConfigSection *section) {
    assert(section);
    assert(!sectionByName_.contains(section->Name()));

    sectionByName_.emplace(section->Name(), section);
}

ConfigSection *Config::Section(const std::string &name) const {
    return ValueOr(sectionByName_, name, nullptr);
}

std::vector<ConfigSection *> Config::Sections() const {
    std::vector<ConfigSection *> result;
    for (const auto &[_, section] : sectionByName_)
        result.push_back(section);
    return result;
}

AbstractConfigValue::AbstractConfigValue(ConfigSection *section, const std::string &name): section(section), name(name) {
    assert(section);
    assert(!name.empty());

    section->RegisterValue(this);
}
