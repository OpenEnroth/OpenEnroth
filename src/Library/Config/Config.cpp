#include "Config.h"

#include <cassert>
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>

#include <inicpp.h> // NOLINT: this is not a C system header.

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/MapAccess.h"
#include "Utility/Exception.h"

void Config::load(std::string_view path) {
    if (!std::filesystem::exists(path))
        throw Exception("Config file '{}' doesn't exist", path);

    FileInputStream stream(path);
    load(&stream);
}

void Config::save(std::string_view path) const {
    FileOutputStream stream(path);
    save(&stream);
}

void Config::load(InputStream *stream) {
    // We'd rather handle FS errors on our side.
    std::istringstream stdStream(stream->readAll());

    ini::IniFile ini;
    ini.decode(stdStream); // This can throw.

    for (const auto &[sectionName, iniSection] : ini)
        if (ConfigSection *section = this->section(sectionName))
            for (const auto &[entryName, iniValue] : iniSection)
                if (AnyConfigEntry *entry = section->entry(entryName))
                    entry->setString(iniValue.as<std::string_view>());
}

void Config::save(OutputStream *stream) const {
    ini::IniFile ini;
    for (ConfigSection *section : sections())
        for (AnyConfigEntry *entry : section->entries())
            ini[section->name()][entry->name()] = entry->string();

    std::ostringstream stdStream;
    ini.encode(stdStream); // This can throw.

    // Same here - we'd rather handle FS errors on our side.
    stream->write(stdStream.str());
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
