#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time/Timer.h"
#include "Engine/Engine.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIQuickReference.h"

#include "Media/Audio/AudioPlayer.h"

GraphicsImage *ui_game_quickref_background = nullptr;

GUIWindow_QuickReference::GUIWindow_QuickReference() : GUIWindow(WINDOW_QuickReference, {0, 0}, render->GetRenderDimensions()) {
    // 004304E7 Game_EventLoop --- part
    pEventTimer->setPaused(true);
    current_screen_type = SCREEN_QUICK_REFERENCE;

    // paperdoll_dbrds[2] = assets->GetImage_16BitAlpha(L"BUTTEXI1");

    if (!ui_game_quickref_background)
        ui_game_quickref_background = assets->getImage_ColorKey("quikref");

    pBtn_ExitCancel = CreateButton({0x187u, 0x13Cu}, {0x4Bu, 0x21u}, 1, 0, UIMSG_Escape, 0,
                                   Io::InputAction::Invalid, localization->GetString(LSTR_EXIT_DIALOGUE), {ui_buttdesc2});
}

void GUIWindow_QuickReference::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    // {
    //     GameUI_QuickRef_Draw();
    // }

    //----- (0041A57E) --------------------------------------------------------
    // void GameUI_QuickRef_Draw()

    Color pTextColor;
    int pFontHeight = assets->pFontArrus->GetHeight() + 1;

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, ui_game_quickref_background);

    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, 18}, colorTable.White, localization->GetString(LSTR_NAME), 60, 0);
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, 47}, colorTable.White, localization->GetString(LSTR_LEVEL), 60, 0);
    int pY = pFontHeight + 47;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_CLASS), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_HP), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_SP), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_AC), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_ATTACK), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_DMG), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_SHOOT), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_DMG), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_SKILLS), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_POINTS), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_COND), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->GetString(LSTR_QSPELL), 60, 0);

    int pX = 89;
    for (Character &player : pParty->pCharacters) {
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, 18}, ui_character_header_text_color, player.name, 84, 0);

        pTextColor = (player.GetActualLevel() <= player.GetBaseLevel()) ? player.GetExperienceDisplayColor() : ui_character_bonus_text_color;
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, 47}, pTextColor, fmt::format("{}", player.GetActualLevel()), 84, 0);

        pY = pFontHeight + 47;
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, localization->GetClassName(player.classType), 84, 0);
        pY += pFontHeight;

        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player.health, player.GetMaxHealth());
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, pTextColor, fmt::format("{}", player.health), 84, 0);
        pY += pFontHeight;

        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player.mana, player.GetMaxMana());
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, pTextColor, fmt::format("{}", player.mana), 84, 0);
        pY += pFontHeight;

        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player.GetActualAC(), player.GetBaseAC());
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, pTextColor, fmt::format("{}", player.GetActualAC()), 84, 0);
        pY += pFontHeight;

        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, fmt::format("{:+}", player.GetActualAttack(false)), 84, 0);
        pY += pFontHeight;

        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, player.GetMeleeDamageString(), 84, 0);
        pY += pFontHeight;

        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, fmt::format("{:+}", player.GetRangedAttack()), 84, 0);
        pY += pFontHeight;

        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, player.GetRangedDamageString(), 84, 0);
        pY += pFontHeight;

        int pSkillsCount = 0;
        for (CharacterSkillType j : allVisibleSkills()) {
            if (player.pActiveSkills[j]) {
                ++pSkillsCount;
            }
        }
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, fmt::format("{}", pSkillsCount), 84, 0);
        pY += pFontHeight;

        pTextColor = player.uSkillPoints ? ui_character_bonus_text_color : ui_character_default_text_color;
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, pTextColor, fmt::format("{}", player.uSkillPoints), 84, 0);
        pY += pFontHeight;

        pTextColor = GetConditionDrawColor(player.GetMajorConditionIdx());
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, pTextColor, localization->GetCharacterConditionName(player.GetMajorConditionIdx()), 84, 0);
        pY += pFontHeight;

        std::string pText = (player.uQuickSpell != SPELL_NONE) ? pSpellStats->pInfos[player.uQuickSpell].pShortName : localization->GetString(LSTR_NONE);
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, pText, 84, 0);

        pX += 94;
    }

    if (pParty->GetPartyReputation() >= 0) {
        pTextColor = (pParty->GetPartyReputation() <= 5) ? ui_character_default_text_color : ui_character_bonus_text_color_neg;
    } else {
        pTextColor = ui_character_bonus_text_color;
    }

    std::string rep = fmt::format("{}: {::}{}\f00000", localization->GetString(LSTR_REPUTATION), pTextColor.tag(), GetReputationString(pParty->GetPartyReputation()));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {22, 323}, colorTable.White, rep);
    std::string fame = fmt::format("\r261{}: {}", localization->GetString(LSTR_FAME), pParty->getPartyFame());
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {0, 323}, colorTable.White, fame);
}
