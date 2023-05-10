#include "Engine/Graphics/DecorationList.h"

#include <cstdlib>

#include "Engine/Engine.h"
#include "Engine/Serialization/LegacyImages.h"
#include "Level/Decoration.h"
#include "Sprites.h"

DecorationList *pDecorationList;

//----- (0045864C) --------------------------------------------------------
void DecorationList::FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8) {
    pDecorations.clear();

    if (data_mm6)
        deserialize(data_mm6, appendVia<DecorationDesc_MM6>(&pDecorations));
    if (data_mm7)
        deserialize(data_mm7, appendVia<DecorationDesc_MM7>(&pDecorations));
    if (data_mm8)
        deserialize(data_mm8, appendVia<DecorationDesc_MM7>(&pDecorations));

    assert(!pDecorations.empty());
}

void DecorationList::InitializeDecorationSprite(unsigned int uDecID) {
    pSpriteFrameTable->InitializeSprite(this->pDecorations[uDecID].uSpriteID);
}

uint16_t DecorationList::GetDecorIdByName(std::string_view pName) {
    if (pName.empty())
        return 0;

    if (pDecorations.size() > 1) {
        for (uint uID = 1; uID < pDecorations.size(); ++uID) {
            if (iequals(pName, pDecorations[uID].pName))
                return uID;
        }
    }

    return 0;
}

void RespawnGlobalDecorations() {
    engine->_persistentVariables.decorVars.fill(0);

    uint decorEventIdx = 0;
    for (uint i = 0; i < pLevelDecorations.size(); ++i) {
        LevelDecoration *decor = &pLevelDecorations[i];

        if (!decor->uEventID) {
            if (decor->IsInteractive()) {
                if (decorEventIdx < 124) {
                    decor->eventVarId = decorEventIdx;
                    engine->_persistentVariables.decorVars[decorEventIdx++] = decor->GetGlobalEvent();
                }
            }
        }
    }
}
