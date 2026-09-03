#include "Engine/Tables/DecorationTable.h"

#include "Engine/Graphics/Sprites.h"

#include "Utility/String/Ascii.h"

DecorationTable *pDecorationTable;

void DecorationTable::initializeSprite(DecorationId id) {
    pSpriteFrameTable->InitializeSprite(decorations[std::to_underlying(id)].uSpriteID);
}

DecorationId DecorationTable::decorationId(std::string_view name) {
    if (name.empty())
        return DECORATION_NULL;

    if (decorations.size() > 1) {
        for (unsigned uID = 1; uID < decorations.size(); ++uID) {
            if (ascii::noCaseEquals(name, decorations[uID].internalName))
                return static_cast<DecorationId>(uID);
        }
    }

    return DECORATION_NULL;
}
