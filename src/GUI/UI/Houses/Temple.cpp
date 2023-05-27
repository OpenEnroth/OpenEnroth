#include <string>

#include "GUI/UI/Houses/Temple.h"

#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIGame.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"

#include "Engine/Localization.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/LocationInfo.h"
#include "Engine/Graphics/LocationFunctions.h"
#include "Engine/Spells/CastSpellInfo.h"

void TempleDialog() {
    int pPrice;                   // edi@1
    int pTextHeight;              // eax@11
    Color pTextColor;  // ax@21
    LocationInfo *ddm;          // edi@29
    GUIButton *pButton;           // edi@64
    uint8_t index;        // [sp+1B7h] [bp-Dh]@64
    int v64;                      // [sp+1B8h] [bp-Ch]@6
    unsigned int pCurrentItem;    // [sp+1BCh] [bp-8h]@6
    int all_text_height;          // [sp+1C0h] [bp-4h]@6

    GUIWindow tample_window = *window_SpeakInHouse;
    tample_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    tample_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    tample_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter()) {  // avoid nzi
        pParty->setActiveToFirstCanAct();
    }

    pPrice = PriceCalculator::templeHealingCostForPlayer(&pParty->activeCharacter(),
                                                         buildingTable[window_SpeakInHouse->wData.val - 1].fPriceMultiplier);
    if (dialog_menu_id == DIALOGUE_MAIN) {
        index = 1;
        pButton = pDialogueWindow->GetControl(
            pDialogueWindow->pStartingPosActiveItem);
        pButton->uHeight = 0;
        pButton->uY = 0;
        if (pParty->activeCharacter().IsPlayerHealableByTemple()) {
            static std::string shop_option_container;
            shop_option_container =
                fmt::format("{} {} {}",
                    localization->GetString(LSTR_HEAL), pPrice,
                    localization->GetString(LSTR_GOLD));
            pShopOptions[0] = shop_option_container.c_str();
            index = 0;
        }
        pShopOptions[1] = localization->GetString(LSTR_DONATE);
        pShopOptions[2] = localization->GetString(LSTR_LEARN_SKILLS);
        all_text_height = 0;
        if (index < pDialogueWindow->pNumPresenceButton) {
            uint i = index;
            for (uint j = index; j < pDialogueWindow->pNumPresenceButton; ++j) {
                all_text_height += pFontArrus->CalcTextHeight(
                    pShopOptions[1 * i], tample_window.uFrameWidth, 0);
                i++;
            }
        }
        v64 = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - (signed int)all_text_height) /
            (pDialogueWindow->pNumPresenceButton - index);
        if (v64 > SIDE_TEXT_BOX_MAX_SPACING) v64 = SIDE_TEXT_BOX_MAX_SPACING;
        all_text_height =
            (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - v64 * (pDialogueWindow->pNumPresenceButton - index) -
            (signed int)all_text_height) /
            2 -
            v64 / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
        if (index + pDialogueWindow->pStartingPosActiveItem <
            pDialogueWindow->pStartingPosActiveItem +
            pDialogueWindow->pNumPresenceButton) {
            uint i = index;
            for (pCurrentItem = index + pDialogueWindow->pStartingPosActiveItem;
                (signed int)pCurrentItem <
                pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                ++pCurrentItem) {
                pButton = pDialogueWindow->GetControl(pCurrentItem);
                pButton->uY = v64 + all_text_height;
                pTextHeight = pFontArrus->CalcTextHeight(
                    pShopOptions[1 * i], tample_window.uFrameWidth, 0);
                pButton->uHeight = pTextHeight;
                pButton->uW = pButton->uY + pTextHeight - 1 + 6;
                all_text_height = pButton->uW;
                pTextColor = colorTable.PaleCanary;
                if (pDialogueWindow->pCurrentPosActiveItem != index + 2)
                    pTextColor = colorTable.White;
                tample_window.DrawTitleText(pFontArrus, 0, pButton->uY,
                    pTextColor, pShopOptions[1 * i], 3);
                i++;
                index++;
            }
        }
        return;
    }
    //-------------------------------------------------
    if (dialog_menu_id == DIALOGUE_TEMPLE_HEAL) {
        if (!pParty->activeCharacter().IsPlayerHealableByTemple()) return;
        if (pParty->GetGold() < pPrice) {
            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
            PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_NotEnoughMoney);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        pParty->TakeGold(pPrice);

        pParty->activeCharacter().conditions.ResetAll();
        pParty->activeCharacter().health =
            pParty->activeCharacter().GetMaxHealth();
        pParty->activeCharacter().mana =
            pParty->activeCharacter().GetMaxMana();

        if (window_SpeakInHouse->wData.val != 78 &&
            (window_SpeakInHouse->wData.val <= 80 ||
            window_SpeakInHouse->wData.val > 82)) {
            if (pParty->activeCharacter().conditions.Has(Condition_Zombie)) {
                // если состояние зомби
                pParty->activeCharacter().uCurrentFace =
                    pParty->activeCharacter().uPrevFace;
                pParty->activeCharacter().uVoiceID =
                    pParty->activeCharacter().uPrevVoiceID;
                GameUI_ReloadPlayerPortraits(
                    pParty->activeCharacterIndex() - 1,
                    pParty->activeCharacter().uPrevFace);
            }
            pAudioPlayer->playExclusiveSound(SOUND_heal);
            pParty->activeCharacter().playReaction(SPEECH_TempleHeal);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        if (pParty->activeCharacter().conditions.Has(Condition_Zombie)) {
            // LODWORD(pParty->activeCharacter().pConditions[Condition_Zombie])
            // =
            // LODWORD(pParty->activeCharacter().pConditions[Condition_Zombie]);
        } else {
            if (pParty->activeCharacter().conditions.HasNone({Condition_Eradicated, Condition_Petrified, Condition_Dead})) {
                pAudioPlayer->playExclusiveSound(SOUND_heal);
                pParty->activeCharacter().playReaction(SPEECH_TempleHeal);
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }
            pParty->activeCharacter().uPrevFace =
                pParty->activeCharacter().uCurrentFace;
            pParty->activeCharacter().uPrevVoiceID =
                pParty->activeCharacter().uVoiceID;
            pParty->activeCharacter().SetCondition(Condition_Zombie, 1);
            pParty->activeCharacter().uVoiceID =
                (pParty->activeCharacter().GetSexByVoice() != 0) + 23;
            pParty->activeCharacter().uCurrentFace =
                (pParty->activeCharacter().GetSexByVoice() != 0) + 23;
            GameUI_ReloadPlayerPortraits(
                pParty->activeCharacterIndex() - 1,
                (pParty->activeCharacter().GetSexByVoice() != 0) + 23);
            pParty->activeCharacter().conditions.Set(Condition_Zombie, pParty->GetPlayingTime());
            // v39 = (GUIWindow *)HIDWORD(pParty->uTimePlayed);
        }
        // HIDWORD(pParty->activeCharacter().pConditions[Condition_Zombie]) =
        // (int)v39;
        pParty->activeCharacter().conditions.Set(Condition_Zombie, pParty->GetPlayingTime());
        pAudioPlayer->playExclusiveSound(SOUND_heal);
        pParty->activeCharacter().playReaction(SPEECH_TempleHeal);
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        return;
    }
    //---------------------------------------------------
    if (dialog_menu_id == DIALOGUE_TEMPLE_DONATE) {
        pPrice = buildingTable[window_SpeakInHouse->wData.val - 1].fPriceMultiplier;
        if (pParty->GetGold() >= pPrice) {
            pParty->TakeGold(pPrice);
            ddm = &currentLocationInfo();

            if (ddm->reputation > -5) {
                ddm->reputation = ddm->reputation - 1;
                if (ddm->reputation - 1 < -5) ddm->reputation = -5;
            }
            if ((uint8_t)byte_F8B1EF[pParty->activeCharacterIndex()] == pParty->uCurrentDayOfMonth % 7) {
                if (ddm->reputation <= -5) {
                    pushTempleSpell(SPELL_AIR_WIZARD_EYE);
                }
                if (ddm->reputation <= -10) {
                    pushTempleSpell(SPELL_SPIRIT_PRESERVATION);
                }
                if (ddm->reputation <= -15) {
                    pushTempleSpell(SPELL_BODY_PROTECTION_FROM_MAGIC);
                }
                if (ddm->reputation <= -20) {
                    pushTempleSpell(SPELL_LIGHT_HOUR_OF_POWER);
                }
                if (ddm->reputation <= -25) {
                    pushTempleSpell(SPELL_LIGHT_DAY_OF_PROTECTION);
                }
            }
            ++byte_F8B1EF[pParty->activeCharacterIndex()];
            pParty->activeCharacter().playReaction(SPEECH_TempleDonate);
            GameUI_SetStatusBar(LSTR_THANK_YOU);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
        PlayHouseSound(window_SpeakInHouse->wData.val,
            HouseSound_NotEnoughMoney);
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        return;
    }
    //------------------------------------------------
    if (dialog_menu_id == DIALOGUE_LEARN_SKILLS) {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            all_text_height = 0;
            v64 = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(),
                                                              buildingTable[window_SpeakInHouse->wData.val - 1]);
            pCurrentItem = 0;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
                i < pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                ++i) {
                auto skill = GetLearningDialogueSkill(
                    (DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param
                );
                if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE
                    && !pParty->activeCharacter().pActiveSkills[skill]) {
                    all_text_height += pFontArrus->CalcTextHeight(
                        localization->GetSkillName(skill),
                        tample_window.uFrameWidth, 0);
                    ++pCurrentItem;
                }
            }

            SkillTrainingDialogue(&tample_window, pCurrentItem, all_text_height, v64);
        }
    }
}
