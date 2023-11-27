#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Config.h"

struct ConfigPatchEntry {
    ConfigPatchEntry() = default;
    ConfigPatchEntry(std::string section, std::string key, std::string value): section(std::move(section)), key(std::move(key)), value(std::move(value)) {}

    std::string section;
    std::string key;
    std::string value;
};

/**
 * Config patch, can be used to dump config state & restore it later.
 *
 * Note that values are stored as string and not as `std::any`. This makes it easy to implement patch serialization,
 * and also means that we don't need to think what happens if the types in config patch and in `AnyConfigValue` don't
 * match (deserialization will throw if they don't).
 */
class ConfigPatch {
 public:
    ConfigPatch() = default;

    static ConfigPatch fromConfig(const Config *config) {
        return fromConfig(config, [] (const ConfigSection *, const AnyConfigEntry *) { return true; });
    }

    template<class Filter>
    static ConfigPatch fromConfig(const Config *config, Filter &&filter) {
        ConfigPatch result;
        for (const ConfigSection *section : config->sections())
            for (const AnyConfigEntry *entry : section->entries())
                if (filter(section, entry))
                    result._entries.emplace_back(section->name(), entry->name(), entry->string());
        return result;
    }

    static ConfigPatch fromEntries(std::vector<ConfigPatchEntry> entries) {
        ConfigPatch result;
        result._entries = std::move(entries);
        return result;
    }

    const std::vector<ConfigPatchEntry> &entries() const {
        return _entries;
    }

    /**
     * Applies this config patch.
     *
     * @param config                    Config to apply this patch to.
     * @throw Exception                 If non-existent keys were found in the patch, and on value deserialization errors.
     */
    void apply(Config *config) const;

 private:
    std::vector<ConfigPatchEntry> _entries;
};
