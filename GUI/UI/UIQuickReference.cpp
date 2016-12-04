#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Timer.h"
#include "Engine/LOD.h"
#include "Engine/texts.h"
#include "Engine/Party.h"
#include "Engine/Graphics/IRender.h"

#include "GUI/GUIFont.h"
#include "GUI/UI/UIQuickReference.h"
#include "GUI/UI/UICharacter.h"

#include "Media/Audio/AudioPlayer.h"


Image *ui_game_quickref_background = nullptr;

GUIWindow_QuickReference::GUIWindow_QuickReference() :
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 5, 0)
{
// 004304E7 Game_EventLoop --- part
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    current_screen_type = SCREEN_QUICK_REFERENCE;

    //paperdoll_dbrds[2] = assets->GetImage_16BitAlpha(L"BUTTEXI1");

    if (!ui_game_quickref_background)
        ui_game_quickref_background = assets->GetImage_16BitColorKey("quikref", 0x7FF);

    pBtn_ExitCancel = CreateButton(
        0x187u, 0x13Cu, 0x4Bu, 0x21u, 1, 0, UIMSG_Escape, 0, 0,
        pGlobalTXT_LocalizationStrings[79],// "Exit"
        ui_buttdesc2,
        0
    ); //, v179);
}

void GUIWindow_QuickReference::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
// {
//     GameUI_QuickRef_Draw();
// }

//----- (0041A57E) --------------------------------------------------------
//void GameUI_QuickRef_Draw()

    unsigned int pTextColor; // eax@7
    unsigned int pX; // [sp+14h] [bp-18h]@2
    unsigned int pY; // edi@9
    int pSkillsCount; // ecx@27
    const char *pText; // eax@38
    int pFontHeight; // [sp+18h] [bp-14h]@1

    pRenderer->DrawTextureAlphaNew(8/640.0f, 8/480.0f, ui_game_quickref_background);

    pFontHeight = LOBYTE(pFontArrus->uFontHeight) + 1;
    for (uint i = 0; i < 4; ++i)
    {
        Player* player = &pParty->pPlayers[i];
        pX = 94 * i + 89;
        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, 18, 0, pGlobalTXT_LocalizationStrings[149], 60, 0);//Name
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 94 * i + 89, 18, ui_character_header_text_color, player->pName, 84, 0);
        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, 47, 0, pGlobalTXT_LocalizationStrings[131], 60, 0); //Уров.
        sprintf(pTmpBuf.data(), "%lu", player->GetActualLevel());
        if (player->GetActualLevel() <= player->GetBaseLevel())
            pTextColor = player->GetExperienceDisplayColor();
        else
            pTextColor = ui_character_bonus_text_color;
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, 47, pTextColor, pTmpBuf.data(), 84, 0);
        pY = pFontHeight + 47;
        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pFontHeight + 47, 0, pGlobalTXT_LocalizationStrings[41], 60, 0);//Класс
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, pClassNames[player->classType], 84, 0);
        pY = pFontHeight + pY;
        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, pGlobalTXT_LocalizationStrings[107], 60, 0);//Здор.
        sprintf(pTmpBuf.data(), "%d", player->sHealth);
        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player->sHealth, player->GetMaxHealth());
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, pTextColor, pTmpBuf.data(), 84, 0);
        pY = pFontHeight + pY;
        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, pGlobalTXT_LocalizationStrings[209], 60, 0);//Мана
        sprintf(pTmpBuf.data(), "%d", player->sMana);
        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player->sMana, player->GetMaxMana());
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, pTextColor, pTmpBuf.data(), 84, 0);
        pY = pFontHeight + pY;
        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, pGlobalTXT_LocalizationStrings[0], 60, 0);//Класс брони
        sprintf(pTmpBuf.data(), "%d", player->GetActualAC());
        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player->GetActualAC(), player->GetBaseAC());
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, pTextColor, pTmpBuf.data(), 84, 0);
        pY = pFontHeight + pY;
        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, pGlobalTXT_LocalizationStrings[18], 60, 0);//Атака
        sprintf(pTmpBuf.data(), "%+d", player->GetActualAttack(false));
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, pTmpBuf.data(), 84, 0);
        pY = pFontHeight + pY;
        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, pGlobalTXT_LocalizationStrings[66], 60, 0);//Повр.
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, player->GetMeleeDamageString(), 84, 0);
        pY = pFontHeight + pY;
        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, pGlobalTXT_LocalizationStrings[203], 60, 0);// Стрелять
        sprintf(pTmpBuf.data(), "%+d", player->GetRangedAttack());
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, pTmpBuf.data(), 84, 0);
        pY = pFontHeight + pY;
        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, pGlobalTXT_LocalizationStrings[66], 60, 0);//Повр.
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, player->GetRangedDamageString(), 84, 0);
        pY = pFontHeight + pY;
        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, pGlobalTXT_LocalizationStrings[205], 60, 0);//Навыки
        pSkillsCount = 0;
        for (uint j = 0; j <= 36; ++j)
        {
            if (player->pActiveSkills[j])
                ++pSkillsCount;
        }
        sprintf(pTmpBuf.data(), "%lu", pSkillsCount);
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, pTmpBuf.data(), 84, 0);
        pY = pFontHeight + pY;
        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, pGlobalTXT_LocalizationStrings[168], 60, 0);//Очки
        sprintf(pTmpBuf.data(), "%lu", player->uSkillPoints);
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, player->uSkillPoints ? ui_character_bonus_text_color : ui_character_default_text_color, pTmpBuf.data(), 84, 0);
        pY = pFontHeight + pY;
        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, pGlobalTXT_LocalizationStrings[45], 60, 0);//Сост.
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, pTextColor, aCharacterConditionNames[player->GetMajorConditionIdx()], 84, 0);
        pY = pFontHeight + pY;
        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, pGlobalTXT_LocalizationStrings[170], 60, 0);//Б.Прим.
        if (player->uQuickSpell)
            pText = pSpellStats->pInfos[player->uQuickSpell].pShortName;
        else
            pText = pGlobalTXT_LocalizationStrings[153];//Нет
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, pText, 84, 0);
    }

    if (pParty->GetPartyReputation() >= 0)
    {
        if (pParty->GetPartyReputation() <= 5)
            pTextColor = ui_character_default_text_color;
        else
            pTextColor = ui_character_bonus_text_color_neg;
    }
    else
        pTextColor = ui_character_bonus_text_color;

    sprintf(pTmpBuf.data(), "%s: \f%05d%s\f00000", pGlobalTXT_LocalizationStrings[180], pTextColor, GetReputationString(pParty->GetPartyReputation()));//Reputation
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 22, 323, 0, pTmpBuf.data(), 0, 0, 0);
    sprintf(pTmpBuf.data(), "\r261%s: %d", pGlobalTXT_LocalizationStrings[84], pParty->GetPartyFame());// Fame Слава
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 0, 323, 0, pTmpBuf.data(), 0, 0, 0);
}