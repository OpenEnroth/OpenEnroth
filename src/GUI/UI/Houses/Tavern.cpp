#include "Tavern.h"

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

#include "Arcomage/Arcomage.h"

#include "Media/MediaPlayer.h"

void GUIWindow_Tavern::mainDialogue() {
    if (!checkIfPlayerCanInteract()) {
        return;
    }

    int pPriceRoom = PriceCalculator::tavernRoomCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);
    int pPriceFood = PriceCalculator::tavernFoodCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);
    int foodNum = houseTable[houseId()].fPriceMultiplier;

    std::vector<std::string> optionsText = {localization->FormatString(LSTR_RENT_ROOM_FOR_D_GOLD, pPriceRoom),
                                            localization->FormatString(LSTR_FILL_PACKS_TO_D_DAYS_FOR_D_GOLD, foodNum, pPriceFood),
                                            localization->GetString(LSTR_LEARN_SKILLS)};

    if (houseId() != HOUSE_TAVERN_EMERALD_ISLAND) {
        optionsText.push_back(localization->GetString(LSTR_PLAY_ARCOMAGE));
    }

    drawOptions(optionsText, colorTable.PaleCanary);
}

void GUIWindow_Tavern::arcomageMainDialogue() {
    if (!checkIfPlayerCanInteract()) {
        return;
    }

    std::vector<std::string> optionsText = {localization->GetString(LSTR_RULES), localization->GetString(LSTR_VICTORY_CONDITIONS)};
    if (pParty->hasItem(ITEM_QUEST_ARCOMAGE_DECK))
        optionsText.push_back(localization->GetString(LSTR_PLAY));

    drawOptions(optionsText, colorTable.PaleCanary);
}

void GUIWindow_Tavern::arcomageRulesDialogue() {
    GUIWindow dialog_window = *this;
    dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    GUIFont *font;
    std::string str = pNPCTopics[354].pText;
    dialog_window.uFrameWidth = game_viewport_width;
    dialog_window.uFrameZ = 452;
    int pTextHeight = assets->pFontArrus->CalcTextHeight(str, dialog_window.uFrameWidth, 12) + 7;
    if (352 - pTextHeight < 8) {
        font = assets->pFontCreate.get();
        pTextHeight = assets->pFontCreate->CalcTextHeight(str, dialog_window.uFrameWidth, 12) + 7;
    } else {
        return; // TODO(captainurist): what's going on here?
    }
    render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
    render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
    DrawText(font, {12, 354 - pTextHeight}, colorTable.White, font->FitTextInAWindow(str, dialog_window.uFrameWidth, 12));
}

void GUIWindow_Tavern::arcomageVictoryCondDialogue() {
    GUIWindow dialog_window = *this;
    dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    std::string label = pNPCTopics[arcomageTopicForTavern(houseId())].pText;
    dialog_window.uFrameWidth = game_viewport_width;
    dialog_window.uFrameZ = 452;
    int pTextHeight = assets->pFontArrus->CalcTextHeight(label, dialog_window.uFrameWidth, 12) + 7;
    render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
    render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
    DrawText(assets->pFontArrus.get(), {12, 354 - pTextHeight}, colorTable.White, assets->pFontArrus->FitTextInAWindow(label, dialog_window.uFrameWidth, 12));
}

void GUIWindow_Tavern::arcomageResultDialogue() {
    GUIWindow dialog_window = *this;
    dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    if (!pParty->hasItem(ITEM_QUEST_ARCOMAGE_DECK)) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return;
    }

    if (pArcomageGame->bGameInProgress == 1) {
        return;
    }
    std::string pText;
    if (pArcomageGame->uGameWinner) {
        if (pArcomageGame->uGameWinner == 1)
            pText = localization->GetString(LSTR_YOU_WON);
        else
            pText = localization->GetString(LSTR_YOU_LOST);
    } else {
        pText = localization->GetString(LSTR_A_TIE);
    }
    int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - assets->pFontArrus->CalcTextHeight(pText, dialog_window.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
    dialog_window.DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.PaleCanary, pText, 3);
}

void GUIWindow_Tavern::restDialogue() {
    int pPriceRoom = PriceCalculator::tavernRoomCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);

    if (pParty->GetGold() >= pPriceRoom) {
        pParty->TakeGold(pPriceRoom);
        playHouseSound(houseId(), HOUSE_SOUND_TAVERN_RENT_ROOM);
        _currentDialogue = DIALOGUE_NULL;
        houseDialogPressEscape();
        playHouseGoodbyeSpeech();
        pMediaPlayer->Unload();

        engine->_messageQueue->addMessageCurrentFrame(UIMSG_RentRoom, std::to_underlying(houseId()), 1);
        Release();
        window_SpeakInHouse = 0;
        return;
    }
    engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
    playHouseSound(houseId(), HOUSE_SOUND_TAVERN_NOT_ENOUGH_GOLD);
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_Tavern::buyFoodDialogue() {
    int pPriceFood = PriceCalculator::tavernFoodCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);

    if ((double)pParty->GetFood() >= houseTable[houseId()].fPriceMultiplier) {
        engine->_statusBar->setEvent(LSTR_YOUR_PACKS_ARE_ALREADY_FULL);
        if (pParty->hasActiveCharacter()) {
            pParty->activeCharacter().playReaction(SPEECH_PACKS_FULL);
        }
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return;
    }
    if (pParty->GetGold() >= pPriceFood) {
        pParty->TakeGold(pPriceFood);
        pParty->SetFood(houseTable[houseId()].fPriceMultiplier);
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
    if (!pParty->hasActiveCharacter()) {  // avoid nzi
        pParty->setActiveToFirstCanAct();
    }

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
        if (pParty->hasItem(ITEM_QUEST_ARCOMAGE_DECK)) {
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
