#include "Engine/Graphics/Overlays.h"

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IRender.h"

#include "Engine/Tables/FrameTableInc.h"
#include "Engine/Tables/IconFrameTable.h"

#include "Engine/TurnEngine/TurnEngine.h"

#include "Engine/Serialization/LegacyImages.h"
#include "Engine/Serialization/Deserializer.h"

#include "GUI/GUIWindow.h"

#include "Sprites.h"


struct OtherOverlayList *pOtherOverlayList = new OtherOverlayList;  // idb
struct OverlayList *pOverlayList = new OverlayList;

// inlined
//----- (mm6c::0045BD50) --------------------------------------------------
void OtherOverlayList::Reset() {
    for (uint i = 0; i < 50; ++i) pOverlays[i].Reset();
}

//----- (004418B1) --------------------------------------------------------
int OtherOverlayList::_4418B1(int a2, int a3, int a4, int a5) { return 0; }

//----- (004418B6) --------------------------------------------------------
int OtherOverlayList::_4418B6(int uOverlayID, int16_t a3, int a4, int a5,
                              int16_t a6) {
    signed int v9;  // esi@6
    int16_t v11;    // dx@11

    for (uint i = 0; i < 50; ++i) {
        if (this->pOverlays[i].field_6 <= 0) {
            this->pOverlays[i].field_0 = 0;
            this->pOverlays[i].screen_space_y = 0;
            this->pOverlays[i].screen_space_x = 0;
            this->pOverlays[i].field_C = a3;
            v9 = 0;
            for (; v9 < (signed int)pOverlayList->pOverlays.size(); ++v9) {
                if (uOverlayID == pOverlayList->pOverlays[v9].uOverlayID) break;
            }
            this->pOverlays[i].field_2 = v9;
            this->pOverlays[i].sprite_frame_time = 0;
            if (a4)
                v11 = a4;
            else
                v11 = 8 * pSpriteFrameTable
                              ->pSpriteSFrames[pOverlayList->pOverlays[v9]
                                                   .uSpriteFramesetID]
                              .uAnimLength;
            this->pOverlays[i].field_6 = v11;
            this->pOverlays[i].field_10 = a5;
            this->pOverlays[i].field_E = a6;
            return true;
        }
    }
    return 0;
}

//----- (00441964) --------------------------------------------------------
void OtherOverlayList::DrawTurnBasedIcon() {
    Icon *frame = nullptr;      // eax@12
    unsigned int v5;  // [sp-8h] [bp-Ch]@4

    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME || !pParty->bTurnBasedModeOn) return;

    if (pTurnEngine->turn_stage == TE_MOVEMENT) {  // все персы отстрелялись(сжатый кулак)
        frame = pIconsFrameTable->GetFrame(
            pIconIDs_Turn[5 - pTurnEngine->uActionPointsLeft / 26],
            pEventTimer->uStartTime);
    } else if (pTurnEngine->turn_stage == TE_WAIT) {
        if (dword_50C998_turnbased_icon_1A)
            v5 = uIconID_TurnStart;  //анимация руки(запуск пошагового режима)
        else
            v5 = uIconID_TurnHour;  //группа ожидает(часы)
        frame = pIconsFrameTable->GetFrame(v5, dword_50C994);
    } else if (pTurnEngine->turn_stage == TE_ATTACK) {  //группа атакует(ладонь)
        frame = pIconsFrameTable->GetFrame(uIconID_TurnStop,
            pEventTimer->uStartTime);
    } else {
        __debugbreak();
        return;
    }
    // if ( render->pRenderD3D )
    render->DrawTextureNew(394 / 640.0f, 288 / 480.0f,
                                frame->GetTexture());
    /*else
      render->DrawTextureIndexedAlpha(0x18Au, 0x120u, v7);*/
    if (dword_50C994 < dword_50C998_turnbased_icon_1A) {
        dword_50C994 += pEventTimer->uTimeElapsed;
        if ((signed int)dword_50C994 >= dword_50C998_turnbased_icon_1A)
            dword_50C998_turnbased_icon_1A = 0;
    }
}

//----- (00458D97) --------------------------------------------------------
void OverlayList::InitializeSprites() {
    for (size_t i = 0; i < pOverlays.size(); ++i)
        pSpriteFrameTable->InitializeSprite(pOverlays[i].uSpriteFramesetID);
}

//----- (00458E08) --------------------------------------------------------
void OverlayList::FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8) {
    pOverlays.clear();

    if (data_mm6)
        BlobDeserializer(data_mm6).ReadLegacyVector<OverlayDesc_MM7>(&pOverlays, Deserializer::Append);
    if (data_mm7)
        BlobDeserializer(data_mm7).ReadLegacyVector<OverlayDesc_MM7>(&pOverlays, Deserializer::Append);
    if (data_mm8)
        BlobDeserializer(data_mm8).ReadLegacyVector<OverlayDesc_MM7>(&pOverlays, Deserializer::Append);

    assert(!pOverlays.empty());
}

//----- (00458E4F) --------------------------------------------------------
bool OverlayList::FromFileTxt(const char *Args) {
    FILE *v4;         // eax@1
    unsigned int v5;  // esi@3
    void *v7;         // eax@9
    // FILE *v8; // ST0C_4@11
    char *i;                // eax@11
    char Buf[490];               // [sp+10h] [bp-2F0h]@3
    FrameTableTxtLine v18;  // [sp+204h] [bp-FCh]@4
    FrameTableTxtLine v19;  // [sp+280h] [bp-80h]@4
    FILE *File;             // [sp+2FCh] [bp-4h]@1
    unsigned int Argsa;     // [sp+308h] [bp+8h]@3

    pOverlays.clear();
    v4 = fopen(Args, "r");
    File = v4;
    if (!v4) Error("ObjectDescriptionList::load - Unable to open file: %s.");

    v5 = 0;
    Argsa = 0;
    if (fgets(Buf, sizeof(Buf), v4)) {
        do {
            *strchr(Buf, 10) = 0;
            memcpy(&v19, txt_file_frametable_parser(Buf, &v18), sizeof(v19));
            if (v19.uPropCount && *v19.pProperties[0] != 47) ++Argsa;
        } while (fgets(Buf, sizeof(Buf), File));
        v5 = Argsa;
    }
    pOverlays.reserve(v5);

    fseek(File, 0, 0);
    for (i = fgets(Buf, sizeof(Buf), File); i; i = fgets(Buf, sizeof(Buf), File)) {
        *strchr(Buf, 10) = 0;
        memcpy(&v19, txt_file_frametable_parser(Buf, &v18), sizeof(v19));
        if (v19.uPropCount && *v19.pProperties[0] != 47) {
            OverlayDesc &overlay = pOverlays.emplace_back();

            overlay.uOverlayID = atoi(v19.pProperties[0]);
            if (!iequals(v19.pProperties[1], "center")) {
                if (iequals(v19.pProperties[1], "transparent"))
                    overlay.uOverlayType = 2;
                else
                    overlay.uOverlayType = 1;
            } else {
                overlay.uOverlayType = 0;
            }
            overlay.uSpriteFramesetID = pSpriteFrameTable->FastFindSprite(v19.pProperties[2]);
        }
    }
    fclose(File);
    return 1;
}

//----- (0045855F) --------------------------------------------------------
void OtherOverlay::Reset() {
    this->field_0 = 0;
    this->field_2 = 0;
    this->sprite_frame_time = 0;
    this->field_6 = 0;
    this->screen_space_x = 0;
    this->screen_space_y = 0;
    this->field_C = 0;
    this->field_E = 0;
    this->field_10 = 65536;
}

//----- (004584B8) --------------------------------------------------------
OtherOverlay::OtherOverlay() { this->Reset(); }
