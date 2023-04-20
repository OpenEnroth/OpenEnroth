#include "Engine/Graphics/DecorationList.h"

#include <cstdlib>

#include "Engine/Events.h"
#include "Engine/Engine.h"
#include "Level/Decoration.h"
#include "Sprites.h"

DecorationList *pDecorationList;

//----- (0045864C) --------------------------------------------------------
void DecorationList::FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8) {
    size_t num_mm6_decs = data_mm6 ? *(uint32_t*)data_mm6.data() : 0;
    size_t num_mm7_decs = data_mm7 ? *(uint32_t*)data_mm7.data() : 0;
    size_t num_mm8_decs = data_mm8 ? *(uint32_t*)data_mm8.data() : 0;

    size_t uNumDecorations = num_mm6_decs + num_mm7_decs + num_mm8_decs;
    Assert(uNumDecorations);
    Assert(!num_mm8_decs);

    DecorationDesc *decors = (DecorationDesc*)((char*)data_mm7.data() + 4);
    for (size_t i = 0; i < num_mm7_decs; i++) {
        pDecorations.push_back(decors[i]);
    }

    DecorationDesc_mm6 *decors_mm6 = (DecorationDesc_mm6*)((char*)data_mm6.data() + 4);
    for (size_t i = 0; i < num_mm6_decs; ++i) {
        DecorationDesc decor;
        static_cast<DecorationDesc_mm6 &>(decor) = decors_mm6[i];
        decor.uColoredLightRed = 255;
        decor.uColoredLightGreen = 255;
        decor.uColoredLightBlue = 255;
        decor.__padding = 255;
        pDecorations.push_back(decor);
    }

    DecorationDesc *decors_mm8 = (DecorationDesc*)((char*)data_mm8.data() + 4);
    for (size_t i = 0; i < num_mm8_decs; i++) {
        pDecorations.push_back(decors_mm8[i]);
    }
}

void DecorationList::InitializeDecorationSprite(unsigned int uDecID) {
    pSpriteFrameTable->InitializeSprite(this->pDecorations[uDecID].uSpriteID);
}

void DecorationList::ToFile() {
    FILE *file = fopen(MakeDataPath("data", "ddeclist.bin").c_str(), "wb");
    if (file == nullptr) {
        Error("Unable to save ddeclist.bin!", 0);
    }

    uint32_t size = this->pDecorations.size();
    fwrite(&size, 4, 1, file);
    fwrite(&pDecorations[0], sizeof(DecorationDesc), pDecorations.size(), file);
    fclose(file);
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
    mapEventVariables.decorVars.fill(0);

    uint decorEventIdx = 0;
    for (uint i = 0; i < pLevelDecorations.size(); ++i) {
        LevelDecoration *decor = &pLevelDecorations[i];

        if (!decor->uEventID) {
            if (decor->IsInteractive()) {
                if (decorEventIdx < 124) {
                    decor->_idx_in_stru123 = decorEventIdx + 75;
                    mapEventVariables.decorVars[decorEventIdx++] = decor->GetGlobalEvent();
                }
            }
        }
    }
}
