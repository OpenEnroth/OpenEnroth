#include <memory>
#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/GameType.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Timer.h"
#include "Engine/Engine.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIQuickReference.h"

#include "Media/Audio/AudioPlayer.h"

std::shared_ptr<GraphicsImage> ui_game_quickref_background;

GUIWindow_QuickReference::GUIWindow_QuickReference() : GUIWindow(WINDOW_QUICK_REFERENCE, {0, 0}, render->GetRenderDimensions()) {
    // 004304E7 Game_EventLoop --- part
    gameTimer->setPaused(true);
    current_screen_type = SCREEN_QUICK_REFERENCE;

    // paperdoll_dbrds[2] = assets->GetImage_16BitAlpha(L"BUTTEXI1");

    if (!ui_game_quickref_background)
        // "quikref" (icons.lod, 640x367) is MM8's background too.
        ui_game_quickref_background = assets->getImage_ColorKey("quikref");

    // MM8 has no "BUTTESC2". Its Close button is the "c_close_up" plaque
    // (ui_exit_cancel_button_background), 83x30.
    Pointi exitCancelPos = {0x187u, 0x13Cu};
    Sizei exitCancelSize = {0x4Bu, 0x21u};
    std::shared_ptr<GraphicsImage> exitCancelTexture = ui_buttdesc2;
    if (gameType() == GAME_TYPE_MM8) {
        exitCancelPos = {552, 330};
        exitCancelSize = {82, 29};
        exitCancelTexture = ui_exit_cancel_button_background;
    }
    pBtn_ExitCancel = CreateButton(exitCancelPos, exitCancelSize, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0,
                                   INPUT_ACTION_INVALID, localization->str(LSTR_EXIT_DIALOGUE), {exitCancelTexture});
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

    render->DrawQuad2D(ui_game_quickref_background, {8, 8});

    // pBtn_ExitCancel's own vTextures[0] only flashes for one frame on click, so the idle
    // state is drawn every frame for MM8. MM7 uses a different asset (BUTTESC2).
    if (gameType() == GAME_TYPE_MM8)
        render->DrawQuad2D(ui_exit_cancel_button_background, pBtn_ExitCancel->rect.topLeft());

    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, 18}, colorTable.White, localization->str(LSTR_NAME), 60, 0);
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, 47}, colorTable.White, localization->str(LSTR_LEVEL), 60, 0);
    int pY = pFontHeight + 47;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_CLASS), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_HP), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_SP), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_AC), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_ATTACK), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_DMG), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_SHOOT), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_DMG), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_SKILLS), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_POINTS), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_COND), 60, 0);
    pY += pFontHeight;
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {22, pY}, colorTable.White, localization->str(LSTR_QSPELL), 60, 0);

    // Walks the active party, not the fixed `pCharacters` array. MM8's "quikref" art has an 84px
    // label column and 5 character columns at a 108px pitch (borders at x=84,192,300,408,516,624).
    int pX = 89;
    for (int i = 0; i < pParty->activeCharacterCount(); ++i) {
        Character &player = pParty->pCharacters[i];
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, 18}, ui_character_header_text_color, player.name, 99, 0);

        pTextColor = (player.GetActualLevel() <= player.GetBaseLevel()) ? player.GetExperienceDisplayColor() : ui_character_bonus_text_color;
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, 47}, pTextColor, fmt::format("{}", player.GetActualLevel()), 99, 0);

        pY = pFontHeight + 47;
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, localization->className(player.classType), 99, 0);
        pY += pFontHeight;

        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player.health, player.GetMaxHealth());
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, pTextColor, fmt::format("{}", player.health), 99, 0);
        pY += pFontHeight;

        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player.mana, player.GetMaxMana());
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, pTextColor, fmt::format("{}", player.mana), 99, 0);
        pY += pFontHeight;

        pTextColor = UI_GetHealthManaAndOtherQualitiesStringColor(player.GetActualAC(), player.GetBaseAC());
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, pTextColor, fmt::format("{}", player.GetActualAC()), 99, 0);
        pY += pFontHeight;

        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, fmt::format("{:+}", player.GetActualAttack(false)), 99, 0);
        pY += pFontHeight;

        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, player.GetMeleeDamageString(), 99, 0);
        pY += pFontHeight;

        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, fmt::format("{:+}", player.GetRangedAttack()), 99, 0);
        pY += pFontHeight;

        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, player.GetRangedDamageString(), 99, 0);
        pY += pFontHeight;

        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, fmt::format("{}", player.visibleSkillCount()), 99, 0);
        pY += pFontHeight;

        pTextColor = player.uSkillPoints ? ui_character_bonus_text_color : ui_character_default_text_color;
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, pTextColor, fmt::format("{}", player.uSkillPoints), 99, 0);
        pY += pFontHeight;

        pTextColor = GetConditionDrawColor(player.GetMajorConditionIdx());
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, pTextColor, localization->characterConditionName(player.GetMajorConditionIdx()), 99, 0);
        pY += pFontHeight;

        std::string pText = (player.uQuickSpell != SPELL_NONE) ? pSpellStats->pInfos[player.uQuickSpell].pShortName : localization->str(LSTR_NONE);
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {pX, pY}, colorTable.White, pText, 99, 0);

        pX += 108;
    }

    if (pParty->GetPartyReputation() >= 0) {
        pTextColor = (pParty->GetPartyReputation() <= 5) ? ui_character_default_text_color : ui_character_bonus_text_color_neg;
    } else {
        pTextColor = ui_character_bonus_text_color;
    }

    // The original prints the numeric reputation value too ("Reputation: 0 Neutral").
    std::string rep = fmt::format("{}: {::}{} {}\f00000", localization->str(LSTR_REPUTATION), pTextColor.tag(), pParty->GetPartyReputation(), GetReputationString(pParty->GetPartyReputation()));
    GUIWindow::DrawText(assets->pFontArrus.get(), {22, 323}, colorTable.White, rep, pGUIWindow_CurrentMenu->frameRect);
    std::string fame = fmt::format("\r261{}: {}", localization->str(LSTR_FAME), pParty->fame());
    GUIWindow::DrawText(assets->pFontArrus.get(), {0, 323}, colorTable.White, fame, pGUIWindow_CurrentMenu->frameRect);
}
