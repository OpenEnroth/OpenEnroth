#include "ConfigSection.h"

#include <cassert>

#include "Utility/MapAccess.h"
#include "Config.h"
#include "Library/Config/AnyConfigEntry.h"

ConfigSection::ConfigSection(Config *config, const std::string &name): _config(config), _name(name) {
    assert(config);
    assert(!name.empty());

    config->registerSection(this);
}

void ConfigSection::registerEntry(AnyConfigEntry *entry) {
    assert(entry);
    assert(!_entryByName.contains(entry->name()));

    _entryByName.emplace(entry->name(), entry);
}

AnyConfigEntry *ConfigSection::entry(const std::string &name) const {
    return valueOr(_entryByName, name, nullptr);
}

std::vector<AnyConfigEntry *> ConfigSection::entries() const {
    std::vector<AnyConfigEntry *> result;
    for(const auto &[_, entry] : _entryByName)
        result.push_back(entry);
    return result;
}
