#include "Config.h"

#include <cassert>
#include <string>
#include <vector>
#include <sstream>

#include <inicpp.h> // NOLINT: this is not a C system header.

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/MapAccess.h"
#include "Utility/String/Format.h"
#include "Utility/String/Wrap.h"

void Config::load(std::string_view path) {
    FileInputStream stream(path); // Will throw if file doesn't exist.
    load(&stream);
}

void Config::save(std::string_view path) const {
    FileOutputStream stream(path);
    save(&stream);
}

void Config::load(InputStream *stream) {
    std::istringstream stdStream(stream->readAll());

    ini::IniFile ini;
    ini.setCommentChar(';'); // Use ini comment char, not '#'.
    ini.decode(stdStream); // This can throw.

    for (const auto &[sectionName, iniSection] : ini)
        if (ConfigSection *section = this->section(sectionName))
            for (const auto &[entryName, iniValue] : iniSection)
                if (AnyConfigEntry *entry = section->entry(entryName))
                    entry->setString(iniValue.as<std::string_view>());
}

void Config::save(OutputStream *stream) const {
    // ini::IniFile doesn't support comments so we just write things out ourselves.
    for (ConfigSection *section : sections()) {
        stream->write(fmt::format("[{}]\n", section->name()));

        for (AnyConfigEntry *entry : section->entries()) {
            if (!entry->description().empty())
                for (const std::string &line : wrapText(entry->description(), 78))
                    stream->write(fmt::format("; {}\n", line));
            stream->write(fmt::format("; Default is '{}'.\n", entry->defaultString()));
            stream->write(fmt::format("{} = {}\n", entry->name(), entry->string()));
            stream->write("\n");
        }

        stream->write("\n");
    }
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
