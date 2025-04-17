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
        "locateEntry", sol::overload(&locateConfigEntry1, &locateConfigEntry2),
        "setEntryValue", &setEntryValue,
        "resetEntryValue", resetEntryValue,
        "toggleEntryValue", toggleEntryValue,
        "entryValue", &entryValue,
        "entryPath", &entryPath,
        "listEntries", sol::as_function([solState] (std::optional<std::string_view> sectionName, std::optional<std::string_view> filter) mutable {
            sol::table result = solState.create_table();
            for (AnyConfigEntry *entry : listEntries(sectionName ? *sectionName : "", filter ? *filter : "")) {
                result.add(solState.create_table_with(
                    "section", entry->section()->name(),
                    "name", entry->name(),
                    "description", entry->description(),
                    "value", entry->string(),
                    "default", entry->defaultString()
                ));
            }
            return result;
        })
    );
}

AnyConfigEntry *ConfigBindings::locateConfigEntry1(std::string_view entryName) {
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

AnyConfigEntry *ConfigBindings::locateConfigEntry2(std::string_view sectionName, std::string_view entryName) {
    ConfigSection *section = engine->config->section(sectionName);
    if (!section)
        throw Exception("Can't find config section '{}'", sectionName);

    AnyConfigEntry *entry = section->entry(entryName);
    if (!entry)
        throw Exception("Can't find config entry '{}.{}'", sectionName, entryName);

    return entry;
}

void ConfigBindings::setEntryValue(AnyConfigEntry *entry, std::string_view value) {
    if (!entry)
        throw Exception("Can't set value of a null config entry.");
    entry->setString(value);
}

void ConfigBindings::resetEntryValue(AnyConfigEntry *entry) {
    if (!entry)
        throw Exception("Can't reset value of a null config entry.");
    entry->reset();
}

void ConfigBindings::toggleEntryValue(AnyConfigEntry *entry) {
    if (!entry)
        throw Exception("Can't toggle value of a null config entry.");
    if (entry->type() != typeid(bool))
        throw Exception("Can't toggle value of a non-boolean config entry.");
    entry->setValue(!std::any_cast<bool>(entry->value()));
}

std::string ConfigBindings::entryValue(AnyConfigEntry *entry) {
    if (!entry)
        throw Exception("Can't get value of a null config entry.");
    return entry->string();
}

std::string ConfigBindings::entryPath(AnyConfigEntry *entry) {
    if (!entry)
        throw Exception("Can't get path of a null config entry.");
    if (!entry->section())
        throw Exception("Can't get path of an orphaned config entry.");
    return entry->section()->name() + "." + entry->name();
}

std::vector<AnyConfigEntry *> ConfigBindings::listEntries(std::string_view sectionName, std::string_view filter) {
    std::vector<ConfigSection *> sections;
    if (sectionName.empty()) {
        sections = engine->config->sections();
    } else if (ConfigSection *section = engine->config->section(sectionName)) {
        sections = {section};
    } else {
        throw Exception("Can't find config section '{}'", sectionName);
    }

    bool filterIsDefault = filter == "default";
    bool filterIsRegular = filter != "default" && filter != "!default";

    std::vector<AnyConfigEntry *> result;
    for (ConfigSection *section : sections) {
        for (AnyConfigEntry *entry : section->entries()) {
            if (!filterIsRegular && (entry->string() == entry->defaultString()) != filterIsDefault)
                continue;
            if (filterIsRegular && entry->name().find(filter) == std::string::npos && entry->string() != filter)
                continue;
            result.emplace_back(entry);
        }
    }
    return result;
}
