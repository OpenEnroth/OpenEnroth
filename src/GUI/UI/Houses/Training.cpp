#include <string>
#include <vector>
#include <limits>

#include "GUI/UI/Houses/Training.h"

#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"

#include "Engine/Localization.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/Outdoor.h"

IndexedArray<int, HOUSE_TRAINING_HALL_EMERALD_ISLE, HOUSE_TRAINING_HALL_STONE_CITY> trainingHallMaxLevels = {
    {HOUSE_TRAINING_HALL_EMERALD_ISLE, 5},
    {HOUSE_TRAINING_HALL_HARMONDALE, 15},
    {HOUSE_TRAINING_HALL_ERATHIA, 25},
    {HOUSE_TRAINING_HALL_TULAREAN_FOREST, 25},
    {HOUSE_TRAINING_HALL_CELESTE, 200},
    {HOUSE_TRAINING_HALL_PIT, 200},
    {HOUSE_TRAINING_HALL_NIGHON, std::numeric_limits<int>::max()}, // no limit
    {HOUSE_TRAINING_HALL_TATALIA, 50},
    {HOUSE_TRAINING_HALL_AVLEE, 50},
    {HOUSE_TRAINING_HALL_STONE_CITY, 100},
};

std::vector<int> charactersTrainedLevels;

void TrainingDialog(const char *s) {
    uint64_t expForNextLevel;  // edi@3
    int v8;               // edx@4
    double v9;            // st7@6
    signed int v10;       // esi@6
    int v14;              // esi@14
    int v33;              // eax@36
    unsigned int v36;     // eax@38
    int index;
    int all_text_height;          // eax@68
    int v49;                      // ebx@69
    GUIButton *pButton;           // eax@71
    int pTextHeight;              // eax@71

    GUIWindow training_dialog_window = *window_SpeakInHouse;
    training_dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    training_dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    training_dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter())  // avoid nzi
        pParty->setActiveToFirstCanAct();

    int pPrice = PriceCalculator::trainingCostForPlayer(&pParty->activeCharacter(),
                                                        buildingTable[window_SpeakInHouse->wData.val - 1]);
    expForNextLevel = 1000ull * pParty->activeCharacter().uLevel * (pParty->activeCharacter().uLevel + 1) / 2;
    //-------------------------------------------------------
    all_text_height = 0;
    if (HouseUI_CheckIfPlayerCanInteract()) {
        if (dialog_menu_id == DIALOGUE_MAIN) {
            if (HouseUI_CheckIfPlayerCanInteract()) {
                index = 0;
                pShopOptions[0] = s;  // set first item to fucntion param - this
                                      // always gets overwritten below??
                pShopOptions[1] = localization->GetString(LSTR_LEARN_SKILLS);
                if (pDialogueWindow->pStartingPosActiveItem < pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton) {
                    for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
                        if (pDialogueWindow->GetControl(i)->msg_param == DIALOGUE_TRAINING_HALL_TRAIN) {
                            static std::string shop_option_str_container;
                            if (pParty->activeCharacter().uLevel >= trainingHallMaxLevels[HOUSE_ID(window_SpeakInHouse->wData.val)]) {
                                shop_option_str_container = fmt::format(
                                    "{}\n \n{}",
                                    localization->GetString(LSTR_TEACHER_LEVEL_TOO_LOW),
                                    localization->GetString(LSTR_CANT_TRAIN_FURTHER));
                                pShopOptions[index] = shop_option_str_container.c_str();
                            } else {
                                if (pParty->activeCharacter().experience < expForNextLevel)
                                    shop_option_str_container = localization->FormatString(
                                        LSTR_XP_UNTIL_NEXT_LEVEL,
                                        (uint)(expForNextLevel - pParty->activeCharacter().experience),
                                        pParty->activeCharacter().uLevel + 1);
                                else
                                    shop_option_str_container = localization->FormatString(
                                        LSTR_FMT_TRAIN_LEVEL_D_FOR_D_GOLD,
                                        pParty->activeCharacter().uLevel + 1,
                                        pPrice);
                                pShopOptions[index] = shop_option_str_container.c_str();
                            }
                        }
                        all_text_height += pFontArrus->CalcTextHeight(pShopOptions[index], training_dialog_window.uFrameWidth, 0);
                        ++index;
                    }
                }
                v49 =
                    (2 * (87 - (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - all_text_height) / 2) - all_text_height) /
                    2 -
                    (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - all_text_height) / 2 / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
                if (pDialogueWindow->pStartingPosActiveItem <
                    pDialogueWindow->pStartingPosActiveItem +
                    pDialogueWindow->pNumPresenceButton) {
                    index = 0;
                    for (int i = pDialogueWindow->pStartingPosActiveItem;
                        i < pDialogueWindow->pStartingPosActiveItem +
                        pDialogueWindow->pNumPresenceButton;
                        ++i) {
                        pButton = pDialogueWindow->GetControl(i);
                        pButton->uY = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - all_text_height) / 2 + v49;
                        pTextHeight = pFontArrus->CalcTextHeight(
                            pShopOptions[index],
                            training_dialog_window.uFrameWidth, 0);
                        pButton->uHeight = pTextHeight;
                        pButton->uW = pTextHeight + pButton->uY - 1 + 6;
                        v49 = pButton->uW;
                        Color pTextColor = colorTable.Jonquil;
                        if (pDialogueWindow->pCurrentPosActiveItem != i)
                            pTextColor = colorTable.White;
                        training_dialog_window.DrawTitleText(
                            pFontArrus, 0, pButton->uY, pTextColor,
                            pShopOptions[index], 3);
                        ++index;
                    }
                }
            }
        }
        //------------------------------------------------------------------
        if (dialog_menu_id == DIALOGUE_TRAINING_HALL_TRAIN) {
            std::string label;

            if (!HouseUI_CheckIfPlayerCanInteract()) {
                v33 = pFontArrus->CalcTextHeight(pNPCTopics[122].pText, training_dialog_window.uFrameWidth, 0);
                training_dialog_window.DrawTitleText(pFontArrus, 0, (212 - v33) / 2 + 101, colorTable.Jonquil, pNPCTopics[122].pText, 3);
                pDialogueWindow->pNumPresenceButton = 0;
                return;
            }
            if (pParty->activeCharacter().uLevel < trainingHallMaxLevels[HOUSE_ID(window_SpeakInHouse->wData.val)]) {
                if ((int64_t)pParty->activeCharacter().experience >= expForNextLevel) {
                    if (pParty->GetGold() >= pPrice) {
                        pParty->TakeGold(pPrice);
                        PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_NotEnoughMoney);
                        pParty->activeCharacter().uLevel++;
                        pParty->activeCharacter().uSkillPoints += pParty->activeCharacter().uLevel / 10 + 5;
                        pParty->activeCharacter().health = pParty->activeCharacter().GetMaxHealth();
                        pParty->activeCharacter().mana = pParty->activeCharacter().GetMaxMana();
                        int maxLevelStepsBefore = *std::max_element(charactersTrainedLevels.begin(), charactersTrainedLevels.end());
                        charactersTrainedLevels[pParty->activeCharacterIndex() - 1]++;
                        int maxLevelStepsAfter = *std::max_element(charactersTrainedLevels.begin(), charactersTrainedLevels.end());
                        if (maxLevelStepsAfter > maxLevelStepsBefore) {
                            GameTime trainingTime = GameTime::FromHours(_494820_training_time(pParty->uCurrentHour) + 4).SubtractMinutes(pParty->uCurrentMinute);
                            if (window_SpeakInHouse->wData.val == HOUSE_TRAINING_HALL_PIT || window_SpeakInHouse->wData.val == HOUSE_TRAINING_HALL_NIGHON) {
                                trainingTime += GameTime::FromHours(12);
                            }
                            restAndHeal(trainingTime.AddDays(7));
                            if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
                                pOutdoor->SetFog();
                        }
                        pParty->activeCharacter().playReaction(SPEECH_LevelUp);

                        GameUI_SetStatusBar(
                            LSTR_FMT_S_NOW_LEVEL_D,
                            pParty->activeCharacter().name.c_str(),
                            pParty->activeCharacter().uLevel,
                            pParty->activeCharacter().uLevel / 10 + 5
                        );

                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                        return;
                    }

                    GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                    PlayHouseSound(window_SpeakInHouse->wData.val, (HouseSoundID)4);
                    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                    return;
                }
                label = localization->FormatString(
                    LSTR_XP_UNTIL_NEXT_LEVEL,
                    (unsigned int)(expForNextLevel - pParty->activeCharacter().experience),
                    pParty->activeCharacter().uLevel + 1);
                v36 = (212 - pFontArrus->CalcTextHeight(
                        label, training_dialog_window.uFrameWidth, 0)) / 2 + 88;
            } else {
                label = fmt::format(
                    "{}\n \n{}",
                    localization->GetString(LSTR_TEACHER_LEVEL_TOO_LOW),
                    localization->GetString(LSTR_CANT_TRAIN_FURTHER));
                v36 = (212 - pFontArrus->CalcTextHeight(
                        label, training_dialog_window.uFrameWidth, 0)) / 2 + 101;
            }
            training_dialog_window.DrawTitleText(
                pFontArrus, 0, v36, colorTable.Jonquil, label, 3);

            PlayHouseSound(window_SpeakInHouse->wData.val, (HouseSoundID)3);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
    }

    //-------------------------------------------------------------
    if (dialog_menu_id == DIALOGUE_LEARN_SKILLS) {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(),
                                                                 buildingTable[window_SpeakInHouse->wData.val - 1]);
            index = 0;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
                (signed int)i < pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                ++i) {
                auto skill = GetLearningDialogueSkill(
                    (DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param
                );
                if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE
                    && !pParty->activeCharacter().pActiveSkills[skill]) {
                    all_text_height += pFontArrus->CalcTextHeight(
                        localization->GetSkillName(skill),
                        training_dialog_window.uFrameWidth, 0);
                    ++index;
                }
            }

            SkillTrainingDialogue(&training_dialog_window, index, all_text_height, pPrice);
        }
    }
    return;
}
