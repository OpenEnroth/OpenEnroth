#include "Engine/Tables/DecorationTable.h"

#include "Engine/Graphics/Sprites.h"

#include "Utility/String/Ascii.h"

DecorationTable *pDecorationTable;

void DecorationTable::initializeSprite(DecorationId id) {
    pSpriteFrameTable->InitializeSprite(decorations[std::to_underlying(id)].uSpriteID);
}

DecorationId DecorationTable::decorationId(std::string_view name) const {
    if (name.empty())
        return DECORATION_NULL;

    for (size_t i = 1; i < decorations.size(); ++i)
        if (ascii::noCaseEquals(name, decorations[i].internalName))
            return static_cast<DecorationId>(i);

    return DECORATION_NULL;
}
