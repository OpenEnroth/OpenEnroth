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
    if (!checkIfPlayerCanInteract()) {
        return;
    }

    int pPrice = PriceCalculator::trainingCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);
    uint64_t expForNextLevel = 1000ull * pParty->activeCharacter().uLevel * (pParty->activeCharacter().uLevel + 1) / 2;
    std::string trainText = "";

    if (pParty->activeCharacter().uLevel >= trainingHallMaxLevels[houseId()]) {
        trainText = fmt::format("{}\n \n{}", localization->GetString(LSTR_TEACHER_LEVEL_TOO_LOW), localization->GetString(LSTR_CANT_TRAIN_FURTHER));
    } else {
        if (pParty->activeCharacter().experience < expForNextLevel) {
            uint64_t expDelta = expForNextLevel - pParty->activeCharacter().experience;
            trainText = localization->FormatString(LSTR_XP_UNTIL_NEXT_LEVEL, expDelta, pParty->activeCharacter().uLevel + 1);
        } else {
            trainText = localization->FormatString(LSTR_FMT_TRAIN_LEVEL_D_FOR_D_GOLD, pParty->activeCharacter().uLevel + 1, pPrice);
        }
    }

    std::vector<std::string> optionsText = {trainText, localization->GetString(LSTR_LEARN_SKILLS)};

    drawOptions(optionsText, colorTable.Jonquil);
}

void GUIWindow_Training::trainDialogue() {
    int pPrice = PriceCalculator::trainingCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);
    uint64_t expForNextLevel = 1000ull * pParty->activeCharacter().uLevel * (pParty->activeCharacter().uLevel + 1) / 2;

    if (!checkIfPlayerCanInteract()) {
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        return;
    }

    if (pParty->activeCharacter().uLevel < trainingHallMaxLevels[houseId()]) {
        if (pParty->activeCharacter().experience >= expForNextLevel) {
            if (pParty->GetGold() >= pPrice) {
                pParty->TakeGold(pPrice);
                playHouseSound(houseId(), HOUSE_SOUND_TRAINING_TRAIN);
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
            playHouseSound(houseId(), HOUSE_SOUND_TRAINING_NOT_ENOUGH_GOLD);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
    }

    playHouseSound(houseId(), HOUSE_SOUND_TRAINING_CANT_TRAIN);
    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
    return;
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
