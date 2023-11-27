#include "EngineTraceConfigSnapshot.h"

#include "Library/Config/Config.h"

EngineTraceConfigSnapshot::EngineTraceConfigSnapshot(Config *config) {
    for (ConfigSection *section : config->sections())
        for (AnyConfigEntry *entry : section->entries())
            _values.emplace_back(entry, entry->value());
}

void EngineTraceConfigSnapshot::apply() {
    for (const auto &[entry, value] : _values)
        entry->setValue(value);
}
