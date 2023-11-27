#include "ConfigPatch.h"

#include "Utility/Exception.h"

void ConfigPatch::apply(Config *config) const {
    for (const ConfigPatchEntry &line : _entries) {
        ConfigSection *section = config->section(line.section);
        if (!section)
            throw Exception("Config section '{}' doesn't exist", line.section);

        AnyConfigEntry *entry = section->entry(line.key);
        if (!entry)
            throw Exception("Config entry '{}.{}' doesn't exist", line.section, line.key);

        entry->setString(line.value);
    }
}
