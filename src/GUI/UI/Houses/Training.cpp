#include <string>
#include <vector>
#include <limits>

#include "GUI/UI/Houses/Training.h"

#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"

#include "Engine/Localization.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"

static constexpr IndexedArray<int, HOUSE_FIRST_TRAINING_HALL, HOUSE_LAST_TRAINING_HALL> trainingHallMaxLevels = {
    {HOUSE_TRAINING_HALL_EMERALD_ISLAND, 5},
    {HOUSE_TRAINING_HALL_HARMONDALE, 15},
    {HOUSE_TRAINING_HALL_ERATHIA, 25},
    {HOUSE_TRAINING_HALL_TULAREAN_FOREST, 25},
    {HOUSE_TRAINING_HALL_CELESTE, 200},
    {HOUSE_TRAINING_HALL_PIT, 200},
    {HOUSE_TRAINING_HALL_MOUNT_NIGHON, std::numeric_limits<int>::max()}, // no limit
    {HOUSE_TRAINING_HALL_TATALIA, 50},
    {HOUSE_TRAINING_HALL_AVLEE, 50},
    {HOUSE_TRAINING_HALL_STONE_CITY, 100},
};

void GUIWindow_Training::mainDialogue() {
    if (!checkIfPlayerCanInteract()) {
        return;
    }

    int pPrice = PriceCalculator::trainingCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);
    uint64_t expForNextLevel = 1000ull * pParty->activeCharacter().uLevel * (pParty->activeCharacter().uLevel + 1) / 2;
    std::string trainText = "";

    if (pParty->activeCharacter().uLevel >= trainingHallMaxLevels[houseId()]) {
        trainText = fmt::format("{}\n \n{}", localization->GetString(LSTR_WITH_YOUR_SKILLS_YOU_SHOULD_BE_WORKING), localization->GetString(LSTR_SORRY_BUT_WE_ARE_UNABLE_TO_TRAIN_YOU));
    } else {
        if (pParty->activeCharacter().experience < expForNextLevel) {
            uint64_t expDelta = expForNextLevel - pParty->activeCharacter().experience;
            trainText = localization->FormatString(LSTR_YOU_NEED_D_MORE_EXPERIENCE_TO_TRAIN_TO, expDelta, pParty->activeCharacter().uLevel + 1);
        } else {
            trainText = localization->FormatString(LSTR_TRAIN_TO_LEVEL_D_FOR_D_GOLD, pParty->activeCharacter().uLevel + 1, pPrice);
        }
    }

    std::vector<std::string> optionsText = {trainText, localization->GetString(LSTR_LEARN_SKILLS)};

    drawOptions(optionsText, colorTable.Sunflower);
}

void GUIWindow_Training::trainDialogue() {
    int pPrice = PriceCalculator::trainingCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);
    uint64_t expForNextLevel = 1000ull * pParty->activeCharacter().uLevel * (pParty->activeCharacter().uLevel + 1) / 2;

    if (!checkIfPlayerCanInteract()) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
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
                    Duration trainingTime = timeUntilDawn() + Duration::fromHours(4);
                    if (houseId() == HOUSE_TRAINING_HALL_PIT || houseId() == HOUSE_TRAINING_HALL_MOUNT_NIGHON) {
                        trainingTime += Duration::fromHours(12);
                    }
                    restAndHeal(trainingTime + Duration::fromDays(7));
                    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                        pOutdoor->SetFog();
                    }
                }
                pParty->activeCharacter().playReaction(SPEECH_LEVEL_UP);

                engine->_statusBar->setEvent(LSTR_S_IS_NOW_LEVEL_LU_AND_HAS_EARNED_LU, pParty->activeCharacter().name,
                                    pParty->activeCharacter().uLevel, pParty->activeCharacter().uLevel / 10 + 5);

                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
                return;
            }

            engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
            playHouseSound(houseId(), HOUSE_SOUND_TRAINING_NOT_ENOUGH_GOLD);
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            return;
        }
    }

    playHouseSound(houseId(), HOUSE_SOUND_TRAINING_CANT_TRAIN);
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
    return;
}

GUIWindow_Training::GUIWindow_Training(HouseId houseId) : GUIWindow_House(houseId) {
    _charactersTrainedLevels.resize(pParty->pCharacters.size());
    std::fill(_charactersTrainedLevels.begin(), _charactersTrainedLevels.end(), 0);
}

void GUIWindow_Training::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
    if (IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
    }
}

void GUIWindow_Training::houseSpecificDialogue() {
    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter()) {  // avoid nzi
        pParty->setActiveToFirstCanAct();
    }

    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_TRAINING_HALL_TRAIN:
        trainDialogue();
        break;
      case DIALOGUE_LEARN_SKILLS:
        learnSkillsDialogue(colorTable.Sunflower);
        break;
      default:
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        break;
    }
}

std::vector<DialogueId> GUIWindow_Training::listDialogueOptions() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        return {DIALOGUE_TRAINING_HALL_TRAIN, DIALOGUE_LEARN_SKILLS};
      case DIALOGUE_LEARN_SKILLS:
        return {DIALOGUE_LEARN_ARMSMASTER, DIALOGUE_LEARN_BODYBUILDING};
      default:
        return {};
    }
}

void GUIWindow_Training::updateDialogueOnEscape() {
    if (IsSkillLearningDialogue(_currentDialogue)) {
        _currentDialogue = DIALOGUE_LEARN_SKILLS;
        return;
    }
    if (_currentDialogue == DIALOGUE_MAIN) {
        _currentDialogue = DIALOGUE_NULL;
        return;
    }
    _currentDialogue = DIALOGUE_MAIN;
}
