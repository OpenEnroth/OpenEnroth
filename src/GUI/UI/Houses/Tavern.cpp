#include "Tavern.h"

#include <fmt/format.h>

#include <memory>
#include <string>
#include <vector>

#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"

#include "Engine/Data/HouseEnumFunctions.h"
#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Party.h"
#include "Engine/mm7_data.h"
#include "Engine/Engine.h"
#include "Engine/GameType.h"

#include "Arcomage/Arcomage.h"
#include "Engine/Graphics/Viewport.h"

#include "Media/MediaPlayer.h"

void GUIWindow_Tavern::mainDialogue() {
    if (!checkIfPlayerCanInteract())
        return;

    int pPriceRoom = PriceCalculator::tavernRoomCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);
    int pPriceFood = PriceCalculator::tavernFoodCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);
    int foodNum = PriceCalculator::tavernFoodDaysForHouse(houseTable[houseId()]);

    std::vector<std::string> optionsText = {localization->format(LSTR_RENT_ROOM_FOR_D_GOLD, pPriceRoom),
                                            localization->format(LSTR_FILL_PACKS_TO_D_DAYS_FOR_D_GOLD, foodNum, pPriceFood),
                                            localization->str(LSTR_LEARN_SKILLS)};

    if (houseId() != HOUSE_TAVERN_EMERALD_ISLAND)
        optionsText.push_back(localization->str(LSTR_PLAY_ARCOMAGE));

    drawOptions(optionsText, colorTable.PaleCanary);
}

// MM8 has no Arcomage deck item (651 is the Ground Wyvern Horn). The original plays with the house deck.
bool partyCanPlayArcomage() {
    return gameType() == GAME_TYPE_MM8 || pParty->hasItem(ITEM_QUEST_ARCOMAGE_DECK);
}

void GUIWindow_Tavern::arcomageMainDialogue() {
    if (!checkIfPlayerCanInteract())
        return;

    std::vector<std::string> optionsText = {localization->str(LSTR_RULES), localization->str(LSTR_VICTORY_CONDITIONS)};
    if (partyCanPlayArcomage())
        optionsText.push_back(localization->str(LSTR_PLAY));

    drawOptions(optionsText, colorTable.PaleCanary);
}

void GUIWindow_Tavern::arcomageRulesDialogue() {
    DrawDialoguePanel(pNPCTopics[354].pText);
}

void GUIWindow_Tavern::arcomageVictoryCondDialogue() {
    DrawDialoguePanel(pNPCTopics[arcomageTopicForTavern(houseId())].pText);
}

void GUIWindow_Tavern::arcomageResultDialogue() {
    Recti dialog_window = this->frameRect;
    dialog_window.x = SIDE_TEXT_BOX_POS_X;
    dialog_window.w = SIDE_TEXT_BOX_WIDTH;

    if (!partyCanPlayArcomage()) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return;
    }

    if (pArcomageGame->_gameInProgress == true)
        return;
    std::string pText;
    if (pArcomageGame->_gameWinner) {
        if (pArcomageGame->_gameWinner == 1) {
            pText = localization->str(LSTR_YOU_WON);
        } else {
            pText = localization->str(LSTR_YOU_LOST);
        }
    } else {
        pText = localization->str(LSTR_A_TIE);
    }
    int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - assets->pFontArrus->CalcTextHeight(pText, dialog_window.w, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
    DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.PaleCanary, pText, 3, dialog_window);
}

void GUIWindow_Tavern::restDialogue() {
    int pPriceRoom = PriceCalculator::tavernRoomCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);

    if (pParty->GetGold() >= pPriceRoom) {
        pParty->TakeGold(pPriceRoom);
        playHouseSound(houseId(), HOUSE_SOUND_TAVERN_RENT_ROOM);
        _currentDialogue = DIALOGUE_NULL;
        houseDialogueGoBack();
        playHouseGoodbyeSpeech();
        pMediaPlayer->Unload();

        engine->_messageQueue->addMessageCurrentFrame(UIMSG_RentRoom, std::to_underlying(houseId()), 1);
        return;
    }
    engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
    playHouseSound(houseId(), HOUSE_SOUND_TAVERN_NOT_ENOUGH_GOLD);
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_Tavern::buyFoodDialogue() {
    int pPriceFood = PriceCalculator::tavernFoodCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);
    int foodDays = PriceCalculator::tavernFoodDaysForHouse(houseTable[houseId()]);

    if (pParty->GetFood() >= foodDays) {
        engine->_statusBar->setEvent(LSTR_YOUR_PACKS_ARE_ALREADY_FULL);
        if (pParty->hasActiveCharacter())
            pParty->activeCharacter().playReaction(SPEECH_PACKS_FULL);
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return;
    }
    if (pParty->GetGold() >= pPriceFood) {
        pParty->TakeGold(pPriceFood);
        pParty->SetFood(foodDays);
        playHouseSound(houseId(), HOUSE_SOUND_TAVERN_BUY_FOOD);
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return;
    }
    engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
    playHouseSound(houseId(), HOUSE_SOUND_TAVERN_NOT_ENOUGH_GOLD);
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_Tavern::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
    if (option == DIALOGUE_TAVERN_ARCOMAGE_RESULT) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_PlayArcomage, 0, 0);
    } else if (IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
    }
}

void GUIWindow_Tavern::houseSpecificDialogue() {
    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter())  // avoid nzi
        pParty->setActiveToFirstCanAct();

    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_TAVERN_ARCOMAGE_MAIN:
        arcomageMainDialogue();
        break;
      case DIALOGUE_TAVERN_ARCOMAGE_RULES:
        arcomageRulesDialogue();
        break;
      case DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS:
        arcomageVictoryCondDialogue();
        break;
      case DIALOGUE_TAVERN_ARCOMAGE_RESULT:
        arcomageResultDialogue();
        break;
      case DIALOGUE_TAVERN_REST:
        restDialogue();
        break;
      case DIALOGUE_TAVERN_BUY_FOOD:
        buyFoodDialogue();
        break;
      case DIALOGUE_LEARN_SKILLS:
        learnSkillsDialogue(colorTable.PaleCanary);
        break;
      default:
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        break;
    }
}

std::vector<DialogueId> GUIWindow_Tavern::listDialogueOptions() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        if (houseId() == HOUSE_TAVERN_EMERALD_ISLAND) {
            return {DIALOGUE_TAVERN_REST, DIALOGUE_TAVERN_BUY_FOOD, DIALOGUE_LEARN_SKILLS};
        } else {
            return {DIALOGUE_TAVERN_REST, DIALOGUE_TAVERN_BUY_FOOD, DIALOGUE_LEARN_SKILLS, DIALOGUE_TAVERN_ARCOMAGE_MAIN};
        }
      case DIALOGUE_LEARN_SKILLS:
        return {DIALOGUE_LEARN_STEALING, DIALOGUE_LEARN_TRAP_DISARM, DIALOGUE_LEARN_PERCEPTION};
      case DIALOGUE_TAVERN_ARCOMAGE_MAIN:
        if (partyCanPlayArcomage()) {
            return {DIALOGUE_TAVERN_ARCOMAGE_RULES, DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS, DIALOGUE_TAVERN_ARCOMAGE_RESULT};
        } else {
            return {DIALOGUE_TAVERN_ARCOMAGE_RULES, DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS};
        }
      default:
        return {};
    }
}

void GUIWindow_Tavern::updateDialogueOnEscape() {
    if (IsSkillLearningDialogue(_currentDialogue)) {
        _currentDialogue = DIALOGUE_LEARN_SKILLS;
        return;
    }
    if (_currentDialogue == DIALOGUE_TAVERN_ARCOMAGE_RULES ||
        _currentDialogue == DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS ||
        _currentDialogue == DIALOGUE_TAVERN_ARCOMAGE_RESULT) {
        _currentDialogue = DIALOGUE_TAVERN_ARCOMAGE_MAIN;
        return;
    }
    if (_currentDialogue == DIALOGUE_MAIN) {
        _currentDialogue = DIALOGUE_NULL;
        return;
    }
    _currentDialogue = DIALOGUE_MAIN;
}

// Portrait rows use the "pcNN-01" textures (`Character::uCurrentFace`, 0-based).
static std::shared_ptr<GraphicsImage> innPortraitImage(const Character &character) {
    return assets->getImage_ColorKey(fmt::format("pc{:02}-01", character.uCurrentFace + 1));
}

void GUIWindow_AdventurersInn::releaseInnButtons() {
    for (GUIButton *button : _innButtons) {
        button->Release();
    }
    _innButtons.clear();
}

void GUIWindow_AdventurersInn::rebuildButtonsIfNeeded() {
    if (!_layoutDirty || !pDialogueWindow) {
        return;
    }
    releaseInnButtons();

    int y = SIDE_TEXT_BOX_POS_Y;
    for (size_t slot = 1; slot < pParty->pCharacters.size(); slot++) {
        Character &character = pParty->pCharacters[slot];
        if (character.rosterId == 0) {
            continue; // Not a recruited roster companion - an empty active slot.
        }
        _innButtons.push_back(pDialogueWindow->CreateButton({SIDE_TEXT_BOX_POS_X, y}, {63, 73}, BUTTON_TYPE_NORMAL, 0,
                                                             UIMSG_InnClickActivePortrait, character.rosterId, INPUT_ACTION_INVALID, character.name));
        _innButtons.push_back(pDialogueWindow->CreateButton({SIDE_TEXT_BOX_POS_X + 68, y + 20}, {SIDE_TEXT_BOX_WIDTH - 68, 20}, BUTTON_TYPE_NORMAL, 0,
                                                             UIMSG_InnClickDismiss, character.rosterId, INPUT_ACTION_INVALID,
                                                             localization->format(LSTR_DISMISS_S, character.name)));
        y += 78;
    }
    for (const Character &character : pParty->mm8Roster.members()) {
        _innButtons.push_back(pDialogueWindow->CreateButton({SIDE_TEXT_BOX_POS_X, y}, {63, 73}, BUTTON_TYPE_NORMAL, 0,
                                                             UIMSG_InnClickBenchPortrait, character.rosterId, INPUT_ACTION_INVALID, character.name));
        y += 78;
    }
    pDialogueWindow->setKeyboardControlGroup(static_cast<int>(_innButtons.size()), false, 0, 2);
    _layoutDirty = false;
}

void GUIWindow_AdventurersInn::houseSpecificDialogue() {
    if (!checkIfPlayerCanInteract()) {
        return;
    }

    rebuildButtonsIfNeeded();

    for (GUIButton *button : _innButtons) {
        if (!button->label.empty()) {
            std::shared_ptr<GraphicsImage> portrait;
            if (button->msg == UIMSG_InnClickActivePortrait) {
                for (Character &member : pParty->pCharacters) {
                    if (member.rosterId == static_cast<int>(button->msg_param)) {
                        portrait = innPortraitImage(member);
                        break;
                    }
                }
            } else if (button->msg == UIMSG_InnClickBenchPortrait) {
                for (const Character &member : pParty->mm8Roster.members()) {
                    if (member.rosterId == static_cast<int>(button->msg_param)) {
                        portrait = innPortraitImage(member);
                        break;
                    }
                }
            }
            if (portrait) {
                render->DrawQuad2D(portrait, button->rect.topLeft());
            }
        }
        DrawText(assets->pFontArrus.get(), {button->rect.x + 68, button->rect.y}, colorTable.White, button->label, frameRect);
    }
}

std::vector<DialogueId> GUIWindow_AdventurersInn::listDialogueOptions() {
    // The original has no dialogue option that leads here, the Inn's portrait list is a screen of its own.
    return {};
}

void GUIWindow_AdventurersInn::innClickBenchPortrait(int rosterId) {
    if (!pParty->mm8Roster.isInInn(rosterId)) {
        return;
    }
    _pendingDismissRosterId = 0;
    if (pParty->activateFromInn(rosterId)) {
        _layoutDirty = true;
        return;
    }
    // Active party is full - remember this bench member as the swap source, wait for a click on
    // an active portrait to complete swapWithInn().
    _pendingSwapBenchRosterId = rosterId;
}

void GUIWindow_AdventurersInn::innClickActivePortrait(int rosterId) {
    if (_pendingSwapBenchRosterId == 0) {
        return;
    }
    if (pParty->swapWithInn(_pendingSwapBenchRosterId, rosterId)) {
        _layoutDirty = true;
    }
    _pendingSwapBenchRosterId = 0;
}

void GUIWindow_AdventurersInn::innClickDismiss(int rosterId) {
    if (pParty->bTurnBasedModeOn) {
        // dismissRoster() refuses and sets the status text itself (Party.cpp), so no
        // two-click confirm is needed.
        pParty->dismissRoster(rosterId);
        _pendingDismissRosterId = 0;
        return;
    }

    if (_pendingDismissRosterId != rosterId) {
        // First click - ask for confirmation with the original's own string.
        engine->_statusBar->setEvent(LSTR_CLICK_THE_DISMISS_BUTTON_AGAIN_TO_DISMISS_THIS_CHARACTER);
        _pendingDismissRosterId = rosterId;
        return;
    }

    // Second click on the same companion - actually bench them.
    if (pParty->dismissRoster(rosterId)) {
        _layoutDirty = true;
    }
    _pendingDismissRosterId = 0;
}
