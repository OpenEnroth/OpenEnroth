#include "Engine/Graphics/DecorationList.h"

#include <cstdlib>

#include "Engine/Engine.h"
#include "Level/Decoration.h"

#include "Utility/String/Ascii.h"

#include "Sprites.h"

DecorationList *pDecorationList;

void DecorationList::InitializeDecorationSprite(unsigned int uDecID) {
    pSpriteFrameTable->InitializeSprite(this->pDecorations[uDecID].uSpriteID);
}

uint16_t DecorationList::GetDecorIdByName(std::string_view pName) {
    if (pName.empty())
        return 0;

    if (pDecorations.size() > 1) {
        for (unsigned uID = 1; uID < pDecorations.size(); ++uID) {
            if (ascii::noCaseEquals(pName, pDecorations[uID].name))
                return uID;
        }
    }

    return 0;
}

void RespawnGlobalDecorations() {
    engine->_persistentVariables.decorVars.fill(0);

    unsigned decorEventIdx = 0;
    for (unsigned i = 0; i < pLevelDecorations.size(); ++i) {
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
