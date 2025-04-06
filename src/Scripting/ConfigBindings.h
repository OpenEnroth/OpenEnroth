#pragma once

#include "IBindings.h"

#include <string>
#include <vector>

class AnyConfigEntry;

class ConfigBindings : public IBindings {
 public:
    virtual sol::table createBindingTable(sol::state_view &solState) const override;

 private:
    static void setConfigValue1(std::string_view sectionName, std::string_view entryName, std::string_view value);
    static void setConfigValue2(std::string_view entryName, std::string_view value);
    [[nodiscard]] static std::string configValue1(std::string_view sectionName, std::string_view entryName);
    [[nodiscard]] static std::string configValue2(std::string_view entryName);
    [[nodiscard]] static std::optional<std::string> resetConfigValue1(std::string_view sectionName, std::string_view entryName);
    [[nodiscard]] static std::optional<std::string> resetConfigValue2(std::string_view entryName);

    [[nodiscard]] static AnyConfigEntry *locateEntry1(std::string_view sectionName, std::string_view entryName);
    [[nodiscard]] static AnyConfigEntry *locateEntry2(std::string_view entryName);

    [[nodiscard]] static sol::table listConfigValues(sol::state_view &solState, std::string_view sectionName, std::string_view filter);
    [[nodiscard]] static std::vector<AnyConfigEntry *> findConfigEntries(std::string_view sectionName, std::string_view filter);
    [[nodiscard]] static std::string configEntryTypeName(AnyConfigEntry *entry);
};
