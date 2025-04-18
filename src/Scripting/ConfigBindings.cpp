#include "ConfigBindings.h"

#include <functional>
#include <string>
#include <ranges>
#include <vector>

#include "Engine/Engine.h"

#include "Utility/Exception.h"
#include "Utility/SmallVector.h"

sol::table ConfigBindings::createBindingTable(sol::state_view &solState) const {
    solState.new_usertype<AnyConfigEntry>("ConfigEntry",
        "value", sol::property(&AnyConfigEntry::string, &AnyConfigEntry::setString),
        "default", sol::property(&AnyConfigEntry::defaultString),
        "name", sol::property(&AnyConfigEntry::name),
        "section", sol::property(&ConfigBindings::sectionName),
        "description", sol::property(&AnyConfigEntry::description),
        "path", sol::property(&ConfigBindings::path),
        "reset", sol::as_function(&AnyConfigEntry::reset),
        "toggle", sol::as_function(&ConfigBindings::toggle)
    );

    return solState.create_table_with(
        "entry", sol::overload(&ConfigBindings::entry1, &ConfigBindings::entry2),
        "list", sol::as_function(&ConfigBindings::list)
    );
}

AnyConfigEntry *ConfigBindings::entry1(std::string_view entryName) {
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

AnyConfigEntry *ConfigBindings::entry2(std::string_view sectionName, std::string_view entryName) {
    ConfigSection *section = engine->config->section(sectionName);
    if (!section)
        throw Exception("Can't find config section '{}'", sectionName);

    AnyConfigEntry *entry = section->entry(entryName);
    if (!entry)
        throw Exception("Can't find config entry '{}.{}'", sectionName, entryName);

    return entry;
}

std::vector<AnyConfigEntry *> ConfigBindings::list(std::string_view sectionName, std::string_view filter) {
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

void ConfigBindings::toggle(AnyConfigEntry *entry) {
    if (!entry)
        throw Exception("Can't toggle value of a null config entry.");
    if (entry->type() != typeid(bool))
        throw Exception("Can't toggle value of a non-boolean config entry.");
    entry->setValue(!std::any_cast<bool>(entry->value()));
}

std::string ConfigBindings::path(AnyConfigEntry *entry) {
    if (!entry)
        throw Exception("Can't get path of a null config entry.");
    if (!entry->section())
        throw Exception("Can't get path of an orphaned config entry.");
    return entry->section()->name() + "." + entry->name();
}

[[nodiscard]] std::string ConfigBindings::sectionName(AnyConfigEntry *entry) {
    if (!entry)
        throw Exception("Can't get section name of a null config entry.");
    if (!entry->section())
        throw Exception("Can't get section name of an orphaned config entry.");
    return entry->section()->name();
}
