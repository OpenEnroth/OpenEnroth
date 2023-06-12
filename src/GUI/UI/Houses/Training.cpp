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

void GUIWindow_Training::mainDialogue() {
    GUIWindow training_dialog_window = *this;
    training_dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    training_dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    training_dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    if (checkIfPlayerCanInteract()) {
        int pPrice = PriceCalculator::trainingCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);
        uint64_t expForNextLevel = 1000ull * pParty->activeCharacter().uLevel * (pParty->activeCharacter().uLevel + 1) / 2;
        int index = 0;
        int all_text_height = 0;

        pShopOptions[0] = "";
        pShopOptions[1] = localization->GetString(LSTR_LEARN_SKILLS);
        for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
            if (pDialogueWindow->GetControl(i)->msg_param == DIALOGUE_TRAINING_HALL_TRAIN) {
                static std::string shop_option_str_container; // TODO(Nik-RE-dev): remove static when pShopOptions becomes local arrray of std::string-s.
                if (pParty->activeCharacter().uLevel >= trainingHallMaxLevels[houseId()]) {
                    shop_option_str_container = fmt::format("{}\n \n{}", localization->GetString(LSTR_TEACHER_LEVEL_TOO_LOW),
                                                            localization->GetString(LSTR_CANT_TRAIN_FURTHER));
                    pShopOptions[index] = shop_option_str_container.c_str();
                } else {
                    if (pParty->activeCharacter().experience < expForNextLevel) {
                        uint64_t expDelta = expForNextLevel - pParty->activeCharacter().experience;
                        shop_option_str_container = localization->FormatString(LSTR_XP_UNTIL_NEXT_LEVEL, expDelta, pParty->activeCharacter().uLevel + 1);
                    } else {
                        shop_option_str_container = localization->FormatString(LSTR_FMT_TRAIN_LEVEL_D_FOR_D_GOLD, pParty->activeCharacter().uLevel + 1, pPrice);
                    }
                    pShopOptions[index] = shop_option_str_container.c_str();
                }
            }
            all_text_height += pFontArrus->CalcTextHeight(pShopOptions[index], training_dialog_window.uFrameWidth, 0);
            ++index;
        }
        int spacing = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - all_text_height) / 2;
        int vertPos = 87 - spacing - all_text_height / 2 - spacing / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;

        index = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton; ++i) {
            GUIButton *pButton = pDialogueWindow->GetControl(i);
            pButton->uY = spacing + vertPos;
            int pTextHeight = pFontArrus->CalcTextHeight(pShopOptions[index], training_dialog_window.uFrameWidth, 0);
            pButton->uHeight = pTextHeight;
            pButton->uW = pTextHeight + pButton->uY - 1 + 6;
            vertPos = pButton->uW;
            Color pTextColor = colorTable.Jonquil;
            if (pDialogueWindow->pCurrentPosActiveItem != i) {
                pTextColor = colorTable.White;
            }
            training_dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY, pTextColor, pShopOptions[index], 3);
            ++index;
        }
    }
}

void GUIWindow_Training::trainDialogue() {
    GUIWindow training_dialog_window = *this;
    training_dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    training_dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    training_dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
    std::string label;
    int textHeight;
    int pPrice = PriceCalculator::trainingCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);
    uint64_t expForNextLevel = 1000ull * pParty->activeCharacter().uLevel * (pParty->activeCharacter().uLevel + 1) / 2;

    if (!checkIfPlayerCanInteract()) {
        int height = pFontArrus->CalcTextHeight(pNPCTopics[122].pText, training_dialog_window.uFrameWidth, 0);
        training_dialog_window.DrawTitleText(pFontArrus, 0, (212 - height) / 2 + 101, colorTable.Jonquil, pNPCTopics[122].pText, 3);
        pDialogueWindow->pNumPresenceButton = 0;
        return;
    }
    if (pParty->activeCharacter().uLevel < trainingHallMaxLevels[houseId()]) {
        if (pParty->activeCharacter().experience >= expForNextLevel) {
            if (pParty->GetGold() >= pPrice) {
                pParty->TakeGold(pPrice);
                PlayHouseSound(wData.val, HouseSound_NotEnoughMoney);
                pParty->activeCharacter().uLevel++;
                pParty->activeCharacter().uSkillPoints += pParty->activeCharacter().uLevel / 10 + 5;
                pParty->activeCharacter().health = pParty->activeCharacter().GetMaxHealth();
                pParty->activeCharacter().mana = pParty->activeCharacter().GetMaxMana();
                int maxLevelStepsBefore = *std::max_element(_charactersTrainedLevels.begin(), _charactersTrainedLevels.end());
                _charactersTrainedLevels[pParty->activeCharacterIndex() - 1]++;
                int maxLevelStepsAfter = *std::max_element(_charactersTrainedLevels.begin(), _charactersTrainedLevels.end());
                if (maxLevelStepsAfter > maxLevelStepsBefore) {
                    GameTime trainingTime = timeUntilDawn().AddHours(4);
                    if (houseId() == HOUSE_TRAINING_HALL_PIT || houseId() == HOUSE_TRAINING_HALL_NIGHON) {
                        trainingTime += GameTime::FromHours(12);
                    }
                    restAndHeal(trainingTime.AddDays(7));
                    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                        pOutdoor->SetFog();
                    }
                }
                pParty->activeCharacter().playReaction(SPEECH_LevelUp);

                GameUI_SetStatusBar(LSTR_FMT_S_NOW_LEVEL_D, pParty->activeCharacter().name.c_str(),
                                    pParty->activeCharacter().uLevel, pParty->activeCharacter().uLevel / 10 + 5);

                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }

            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
            PlayHouseSound(wData.val, HouseSound_Goodbye);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        uint64_t expDelta = expForNextLevel - pParty->activeCharacter().experience;
        label = localization->FormatString(LSTR_XP_UNTIL_NEXT_LEVEL, expDelta, pParty->activeCharacter().uLevel + 1);
        textHeight = (212 - pFontArrus->CalcTextHeight(label, training_dialog_window.uFrameWidth, 0)) / 2 + 88;
    } else {
        label = fmt::format("{}\n \n{}", localization->GetString(LSTR_TEACHER_LEVEL_TOO_LOW), localization->GetString(LSTR_CANT_TRAIN_FURTHER));
        textHeight = (212 - pFontArrus->CalcTextHeight(label, training_dialog_window.uFrameWidth, 0)) / 2 + 101;
    }
    training_dialog_window.DrawTitleText(pFontArrus, 0, textHeight, colorTable.Jonquil, label, 3);

    PlayHouseSound(wData.val, HouseSound_Greeting_2);
    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
    return;
}

void GUIWindow_Training::learnSkillsDialogue() {
    GUIWindow training_dialog_window = *this;
    training_dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    training_dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    training_dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
    int all_text_height = 0;

    if (checkIfPlayerCanInteract()) {
        int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);
        int index = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
            auto skill = GetLearningDialogueSkill((DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param);
            if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE &&
                !pParty->activeCharacter().pActiveSkills[skill]) {
                all_text_height += pFontArrus->CalcTextHeight(localization->GetSkillName(skill), training_dialog_window.uFrameWidth, 0);
                ++index;
            }
        }

        SkillTrainingDialogue(&training_dialog_window, index, all_text_height, pPrice);
    }
}

GUIWindow_Training::GUIWindow_Training(HOUSE_ID houseId) : GUIWindow_House(houseId) {
    _charactersTrainedLevels.resize(pParty->pPlayers.size());
    std::fill(_charactersTrainedLevels.begin(), _charactersTrainedLevels.end(), 0);
}

void GUIWindow_Training::houseDialogueOptionSelected(DIALOGUE_TYPE option) {
    if (IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
    }
}

void GUIWindow_Training::houseSpecificDialogue() {
    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter()) {  // avoid nzi
        pParty->setActiveToFirstCanAct();
    }

    switch (dialog_menu_id) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_TRAINING_HALL_TRAIN:
        trainDialogue();
        break;
      case DIALOGUE_LEARN_SKILLS:
        learnSkillsDialogue();
        break;
      default:
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }
}

std::vector<DIALOGUE_TYPE> GUIWindow_Training::listDialogueOptions(DIALOGUE_TYPE option) {
    switch (option) {
      case DIALOGUE_MAIN:
        return {DIALOGUE_TRAINING_HALL_TRAIN, DIALOGUE_LEARN_SKILLS};
      case DIALOGUE_LEARN_SKILLS:
        return {DIALOGUE_LEARN_ARMSMASTER, DIALOGUE_LEARN_BODYBUILDING};
      default:
        return {};
    }
}

DIALOGUE_TYPE GUIWindow_Training::getOptionOnEscape() {
    if (IsSkillLearningDialogue(dialog_menu_id)) {
        return DIALOGUE_LEARN_SKILLS;
    }
    if (dialog_menu_id == DIALOGUE_MAIN) {
        return DIALOGUE_NULL;
    }
    return DIALOGUE_MAIN;
}
