#pragma once

#include "IBindings.h"

#include <string>
#include <vector>

class AnyConfigEntry;

class ConfigBindings : public IBindings {
 public:
    virtual sol::table createBindingTable(sol::state_view &solState) const override;

 private:
    [[nodiscard]] static AnyConfigEntry *locateConfigEntry1(std::string_view entryName);
    [[nodiscard]] static AnyConfigEntry *locateConfigEntry2(std::string_view sectionName, std::string_view entryName);

    static void setEntryValue(AnyConfigEntry *entry, std::string_view value);
    static void resetEntryValue(AnyConfigEntry *entry);
    static void toggleEntryValue(AnyConfigEntry *entry);
    [[nodiscard]] static std::string entryValue(AnyConfigEntry *entry);
    [[nodiscard]] static std::string entryPath(AnyConfigEntry *entry);
    [[nodiscard]] static std::vector<AnyConfigEntry *> listEntries(std::string_view sectionName, std::string_view filter);
};
