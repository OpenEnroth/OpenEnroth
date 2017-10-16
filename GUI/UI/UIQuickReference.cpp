#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/AssetsManager.h"
#include "Engine/Time.h"
#include "Engine/LOD.h"
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
        localization->GetString(79),// "Exit"
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

    render->DrawTextureAlphaNew(8/640.0f, 8/480.0f, ui_game_quickref_background);

    pFontHeight = pFontArrus->GetFontHeight() + 1;
    for (uint i = 0; i < 4; ++i)
    {
        Player* player = &pParty->pPlayers[i];
        pX = 94 * i + 89;
        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, 18, 0, localization->GetString(149), 60, 0);//Name
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 94 * i + 89, 18, ui_character_header_text_color, player->pName, 84, 0);

        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, 47, 0, localization->GetString(131), 60, 0); //Уров.
        if (player->GetActualLevel() <= player->GetBaseLevel())
            pTextColor = player->GetExperienceDisplayColor();
        else
            pTextColor = ui_character_bonus_text_color;
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, 47, pTextColor, StringPrintf("%lu", player->GetActualLevel()), 84, 0);

        pY = pFontHeight + 47;
        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pFontHeight + 47, 0, localization->GetString(41), 60, 0);//Класс
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, localization->GetClassName(player->classType), 84, 0);
        pY = pFontHeight + pY;

        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, localization->GetString(107), 60, 0);//Здор.
        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player->sHealth, player->GetMaxHealth());
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, pTextColor, StringPrintf("%d", player->sHealth), 84, 0);
        pY = pFontHeight + pY;

        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, localization->GetString(209), 60, 0);//Мана
        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player->sMana, player->GetMaxMana());
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, pTextColor, StringPrintf("%d", player->sMana), 84, 0);
        pY = pFontHeight + pY;

        if (i == 0)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, localization->GetString(0), 60, 0);//Класс брони
        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player->GetActualAC(), player->GetBaseAC());
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, pTextColor, StringPrintf("%d", player->GetActualAC()), 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, localization->GetString(18), 60, 0);//Атака
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, StringPrintf("%+d", player->GetActualAttack(false)), 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, localization->GetString(66), 60, 0); // Damage   Повр.
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, player->GetMeleeDamageString(), 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, localization->GetString(203), 60, 0);// Стрелять
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, StringPrintf("%+d", player->GetRangedAttack()), 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, localization->GetString(66), 60, 0);//Повр.
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, player->GetRangedDamageString(), 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, localization->GetString(205), 60, 0);//Навыки
        pSkillsCount = 0;
        for (uint j = 0; j <= 36; ++j)
        {
            if (player->pActiveSkills[j])
                ++pSkillsCount;
        }
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, 0, StringPrintf("%lu", pSkillsCount), 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, localization->GetString(168), 60, 0);//Очки
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, player->uSkillPoints ? ui_character_bonus_text_color : ui_character_default_text_color, StringPrintf("%lu", player->uSkillPoints), 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, localization->GetString(45), 60, 0);//Сост.
        pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, pX, pY, pTextColor, localization->GetCharacterConditionName(player->GetMajorConditionIdx()), 84, 0);
        pY = pFontHeight + pY;

        if (!i)
            pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 22, pY, 0, localization->GetString(170), 60, 0);//Б.Прим.
        if (player->uQuickSpell)
            pText = pSpellStats->pInfos[player->uQuickSpell].pShortName;
        else
            pText = localization->GetString(153);//Нет
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

    auto str1 = StringPrintf("%s: \f%05d%s\f00000", localization->GetString(180), pTextColor, GetReputationString(pParty->GetPartyReputation()).c_str());//Reputation
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 22, 323, 0, str1, 0, 0, 0);
    auto str2 = StringPrintf("\r261%s: %d", localization->GetString(84), pParty->GetPartyFame());// Fame Слава
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 0, 323, 0, str2, 0, 0, 0);
}