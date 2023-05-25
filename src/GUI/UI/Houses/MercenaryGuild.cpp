#include "GUI/UI/Houses/MercenaryGuild.h"

#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"

#include "Engine/Localization.h"
#include "Engine/PriceCalculator.h"

/**
  * @offset 0x004B6478.
  *
  * @brief Mercenary guild in MM6.
  */
void MercenaryGuildDialog() {
    signed int v3;                // esi@1
    short *v6;                       // edi@6
    int all_text_height;          // eax@20
    int pTextHeight;              // eax@29
    int v27;                      // [sp-4h] [bp-80h]@8
    int v32;                      // [sp+6Ch] [bp-10h]@1
    int index;                    // [sp+74h] [bp-8h]@17

    GUIWindow dialog_window = *window_SpeakInHouse;
    dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    /*
     * archiving this code just in case
     * I believe it is 250 gold cost for mercenary guild from mm6 and 100 for all other skill-learning house types in mm6
     * but they aren't used in mm7, so I'm gonna assume 250 gold cost in price calculator
     *
     *  v32 = (uint8_t)(((buildingTable[window_SpeakInHouse->wData.val - 1].uType != BuildingType_MercenaryGuild) - 1) & 0x96) + 100;
     *  v3 = (int64_t)((double)v32 * buildingTable[window_SpeakInHouse->wData.val - 1].fPriceMultiplier);
     *  pPrice = v3 * (100 - PriceCalculator::playerMerchant(&pParty->activeCharacter())) / 100;
     *  if (pPrice < v3 / 3) pPrice = v3 / 3;
     */
    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), buildingTable[window_SpeakInHouse->wData.val - 1]);

    if (dialog_menu_id == DIALOGUE_MAIN) {
        if (!pParty->activeCharacter()._achievedAwardsBits[word_4F0754[2 * window_SpeakInHouse->wData.val]]) {
            // 171 looks like Mercenary Stronghold message from NPCNews.txt in MM6
            pTextHeight = pFontArrus->CalcTextHeight(pNPCTopics[171].pText, dialog_window.uFrameWidth, 0);
            dialog_window.DrawTitleText(pFontArrus, 0, (212 - pTextHeight) / 2 + 101, colorTable.PaleCanary.c16(), pNPCTopics[171].pText, 3);
            pDialogueWindow->pNumPresenceButton = 0;
            return;
        }
        if (!HouseUI_CheckIfPlayerCanInteract()) return;
        all_text_height = 0;
        index = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
            auto skill = GetLearningDialogueSkill((DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param);
            // Was class type / 3
            if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE
                && !pParty->activeCharacter().pActiveSkills[skill]) {
                all_text_height += pFontArrus->CalcTextHeight(localization->GetSkillName(skill), dialog_window.uFrameWidth, 0);
                ++index;
            }
        }


        SkillTrainingDialogue(&dialog_window, index, all_text_height, pPrice);
        return;
    }

    if (HouseUI_CheckIfPlayerCanInteract()) {
        __debugbreak();  // what type of house that even is?
        // pSkillAvailabilityPerClass[8 + v58->uClass][4 + v23]
        // or
        // skillMaxMasteryPerClass[v58->uClass][v23 - 36]
        // or
        // skillMaxMasteryPerClass[v58->uClass - 1][v23 +
        // 1]
        __debugbreak();  // whacky condition - fix
        if (false
            // if ( !*(&byte_4ED94C[37 * v1->uClass / 3] + dword_F8B19C)
            || (v6 = (short *)(&pParty->activeCharacter().uIntelligence +
                dialog_menu_id),
                *(short *)v6)) {
            pAudioPlayer->playUISound(SOUND_error);
        } else {
            if (pParty->GetGold() < pPrice) {
                GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                v27 = 4;
            } else {
                pParty->TakeGold(pPrice);
                *(short *)v6 = 1;
                v27 = 2;
            }
            PlayHouseSound(window_SpeakInHouse->wData.val, (HouseSoundID)v27);
        }
    }
    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
}
