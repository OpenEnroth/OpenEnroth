#include "Engine/Graphics/DecorationList.h"

#include <cstdlib>

#include "Engine/Tables/FrameTableInc.h"
#include "Engine/stru123.h"
#include "Engine/Engine.h"
#include "Level/Decoration.h"
#include "Sprites.h"

#include "Platform/Api.h"
#include "Platform/Path.h"

struct DecorationList *pDecorationList;

//----- (0045864C) --------------------------------------------------------
void DecorationList::FromFile(void *data_mm6, void *data_mm7, void *data_mm8) {
    size_t num_mm6_decs = data_mm6 ? *(uint32_t*)data_mm6 : 0;
    size_t num_mm7_decs = data_mm7 ? *(uint32_t*)data_mm7 : 0;
    size_t num_mm8_decs = data_mm8 ? *(uint32_t*)data_mm8 : 0;

    size_t uNumDecorations = num_mm6_decs + num_mm7_decs + num_mm8_decs;
    Assert(uNumDecorations);
    Assert(!num_mm8_decs);

    DecorationDesc *decors = (DecorationDesc*)((char*)data_mm7 + 4);
    for (size_t i = 0; i < num_mm7_decs; i++) {
        pDecorations.push_back(decors[i]);
    }

    DecorationDesc_mm6 *decors_mm6 = (DecorationDesc_mm6*)((char*)data_mm6 + 4);
    for (size_t i = 0; i < num_mm6_decs; ++i) {
        DecorationDesc decor;
        memcpy(&decor, &decors_mm6[i], sizeof(DecorationDesc_mm6));
        decor.uColoredLightRed = 255;
        decor.uColoredLightGreen = 255;
        decor.uColoredLightBlue = 255;
        decor.__padding = 255;
        pDecorations.push_back(decor);
    }

    DecorationDesc *decors_mm8 = (DecorationDesc*)((char*)data_mm8 + 4);
    for (size_t i = 0; i < num_mm8_decs; i++) {
        pDecorations.push_back(decors_mm8[i]);
    }
}

void DecorationList::InitializeDecorationSprite(unsigned int uDecID) {
    pSpriteFrameTable->InitializeSprite(this->pDecorations[uDecID].uSpriteID);
}

void DecorationList::ToFile() {
    FILE *file = fcaseopen(DATA_PATH "/ddeclist.bin", "wb");
    if (file == nullptr) {
        Error("Unable to save ddeclist.bin!", 0);
    }

    fwrite(this, 4, 1, file);
    fwrite(&pDecorations[0], sizeof(DecorationDesc), pDecorations.size(), file);
    fclose(file);
}

uint16_t DecorationList::GetDecorIdByName(const char *pName) {
    if (pName && pDecorations.size() > 1) {
        for (uint uID = 1; uID < pDecorations.size(); ++uID) {
            if (!_stricmp(pName, pDecorations[uID].pName)) return uID;
        }
    }

    return 0;
}

void RespawnGlobalDecorations() {
    memset(stru_5E4C90_MapPersistVars._decor_events.data(), 0, 125);

    uint decorEventIdx = 0;
    for (uint i = 0; i < uNumLevelDecorations; ++i) {
        LevelDecoration *decor = &pLevelDecorations[i];

        if (!decor->uEventID) {
            if (decor->IsInteractive()) {
                if (decorEventIdx < 124) {
                    decor->_idx_in_stru123 = decorEventIdx + 75;
                    stru_5E4C90_MapPersistVars._decor_events[decorEventIdx++] =
                        decor->GetGlobalEvent();
                }
            }
        }
    }
}
