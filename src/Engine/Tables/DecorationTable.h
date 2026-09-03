#pragma once

#include <string_view>
#include <utility>
#include <vector>

#include "Engine/Data/DecorationData.h"
#include "Engine/Data/DecorationEnums.h"

struct DecorationTable {
    void initializeSprite(DecorationId id);

    /**
     * @param name                      Internal name of a decoration, case-insensitive.
     * @return                          Id of the decoration with the given name, or `DECORATION_NULL` if there is
     *                                  no such decoration.
     */
    DecorationId decorationId(std::string_view name);

    const DecorationData *decoration(DecorationId index) const {
        return &decorations[std::to_underlying(index)];
    }

    std::vector<DecorationData> decorations; // TODO(captainurist): IndexedArray.
};

extern DecorationTable *pDecorationTable;
