#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIQuickReference.h"

#include "Media/Audio/AudioPlayer.h"

#include "Platform/Api.h"
#include "Platform/OSWindow.h"


Image *ui_game_quickref_background = nullptr;

GUIWindow_QuickReference::GUIWindow_QuickReference()
    : GUIWindow(WINDOW_QuickReference, 0, 0, window->GetWidth(), window->GetHeight(), (GUIButton *)5) {
    // 004304E7 Game_EventLoop --- part
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    current_screen_type = CURRENT_SCREEN::SCREEN_QUICK_REFERENCE;

    // paperdoll_dbrds[2] = assets->GetImage_16BitAlpha(L"BUTTEXI1");

    if (!ui_game_quickref_background)
        ui_game_quickref_background =
            assets->GetImage_ColorKey("quikref", render->teal_mask_16);

    pBtn_ExitCancel = CreateButton(
        0x187u, 0x13Cu, 0x4Bu, 0x21u, 1, 0, UIMSG_Escape, 0, GameKey::None,
        localization->GetString(LSTR_DIALOGUE_EXIT),
        {{ui_buttdesc2}}
    );
}

void GUIWindow_QuickReference::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    // {
    //     GameUI_QuickRef_Draw();
    // }

    //----- (0041A57E) --------------------------------------------------------
    // void GameUI_QuickRef_Draw()

    unsigned int pTextColor;  // eax@7
    unsigned int pX;          // [sp+14h] [bp-18h]@2
    unsigned int pY;          // edi@9
    int pSkillsCount;         // ecx@27
    const char *pText;        // eax@38
    int pFontHeight;          // [sp+18h] [bp-14h]@1
    String pTempString;

    render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f,
                                ui_game_quickref_background);

    pFontHeight = pFontArrus->GetHeight() + 1;
    for (uint i = 0; i < 4; ++i) {
        Player *player = &pParty->pPlayers[i];
        pX = 94 * i + 89;
        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(
                pFontArrus, 22, 18, 0, localization->GetString(LSTR_NAME), 60, 0
            );
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 94 * i + 89, 18,
                                               ui_character_header_text_color,
                                               player->pName, 84, 0);

        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, 47, 0,
                                                   localization->GetString(LSTR_LEVEL),
                                                   60, 0);
        if (player->GetActualLevel() <= player->GetBaseLevel())
            pTextColor = player->GetExperienceDisplayColor();
        else
            pTextColor = ui_character_bonus_text_color;
        pTempString = StringPrintf("%lu", player->GetActualLevel());
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, 47, pTextColor,
            pTempString, 84, 0);

        pY = pFontHeight + 47;
        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(
                pFontArrus, 22, pFontHeight + 47, 0, localization->GetString(LSTR_CLASS), 60, 0
            );
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, pY, 0,
            localization->GetClassName(player->classType), 84, 0);
        pY = pFontHeight + pY;

        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(
                pFontArrus, 22, pY, 0, localization->GetString(LSTR_HP), 60, 0
            );
        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(
            player->sHealth, player->GetMaxHealth());
        pTempString = StringPrintf("%d", player->sHealth);
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, pY, pTextColor, pTempString,
            84, 0);
        pY = pFontHeight + pY;

        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(
                pFontArrus, 22, pY, 0, localization->GetString(LSTR_SP), 60, 0
            );
        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(
            player->sMana, player->GetMaxMana());
        pTempString = StringPrintf("%d", player->sMana);
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, pY, pTextColor, pTempString,
            84, 0);
        pY = pFontHeight + pY;

        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0,
                                                   localization->GetString(LSTR_AC),
                                                   60, 0);
        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(
            player->GetActualAC(), player->GetBaseAC());
        pTempString = StringPrintf("%d", player->GetActualAC());
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, pY, pTextColor,
            pTempString, 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0,
                                                   localization->GetString(LSTR_ATTACK),
                                                   60, 0);
        pTempString = StringPrintf("%+d", player->GetActualAttack(false));
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, pY, 0,
            pTempString, 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(
                pFontArrus, 22, pY, 0, localization->GetString(LSTR_DMG), 60, 0
            );
        pTempString = player->GetMeleeDamageString();
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, pY, 0, pTempString, 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0,
                                                   localization->GetString(LSTR_SHOOT),
                                                   60, 0);
        pTempString = StringPrintf("%+d", player->GetRangedAttack());
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, pY, 0,
            pTempString, 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(
                pFontArrus, 22, pY, 0, localization->GetString(LSTR_DMG), 60, 0
            );
        pTempString = player->GetRangedDamageString();
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, pY, 0, pTempString, 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0,
                                                   localization->GetString(LSTR_SKILLS),
                                                   60, 0);
        pSkillsCount = 0;
        for (uint j = 0; j <= 36; ++j) {
            if (player->pActiveSkills[j]) ++pSkillsCount;
        }
        pTempString = StringPrintf("%lu", pSkillsCount);
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, pY, 0, pTempString, 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(
                pFontArrus, 22, pY, 0, localization->GetString(LSTR_POINTS), 60, 0
            );
        pTempString = StringPrintf("%lu", player->uSkillPoints);
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, pY,
            player->uSkillPoints ? ui_character_bonus_text_color
                                 : ui_character_default_text_color,
            pTempString, 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(
                pFontArrus, 22, pY, 0, localization->GetString(LSTR_COND), 60, 0);
        pGUIWindow_CurrentMenu->DrawTextInRect(
            pFontArrus, pX, pY, pTextColor,
            localization->GetCharacterConditionName(
                player->GetMajorConditionIdx()),
            84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(
                pFontArrus, 22, pY, 0, localization->GetString(LSTR_QSPELL), 60, 0
            );
        if (player->uQuickSpell)
            pText = pSpellStats->pInfos[player->uQuickSpell].pShortName;
        else
            pText = localization->GetString(LSTR_NONE);
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, pText, 84, 0);
    }

    if (pParty->GetPartyReputation() >= 0) {
        if (pParty->GetPartyReputation() <= 5)
            pTextColor = ui_character_default_text_color;
        else
            pTextColor = ui_character_bonus_text_color_neg;
    } else {
        pTextColor = ui_character_bonus_text_color;
    }

    auto rep = StringPrintf(
        "%s: \f%05d%s\f00000",
        localization->GetString(LSTR_REPUTATION), pTextColor,
        GetReputationString(pParty->GetPartyReputation()).c_str()
    );
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 22, 323, 0, rep, 0, 0, 0);
    auto fame = StringPrintf(
        "\r261%s: %d", localization->GetString(LSTR_FAME), pParty->GetPartyFame()
    );
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 0, 323, 0, fame, 0, 0, 0);
}
