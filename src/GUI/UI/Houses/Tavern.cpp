#include <string>

#include "GUI/UI/Houses/Tavern.h"

#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"

#include "Engine/Localization.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/IRender.h"

#include "Arcomage/Arcomage.h"

#include "Media/MediaPlayer.h"

void GUIWindow_Tavern::mainDialogue() {
    if (!checkIfPlayerCanInteract())
        return;

    GUIWindow dialog_window = *this;
    dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    int pPriceRoom = PriceCalculator::tavernRoomCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);
    int pPriceFood = PriceCalculator::tavernFoodCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);

    Color color = pDialogueWindow->pCurrentPosActiveItem == 2 ? colorTable.PaleCanary : colorTable.White;
    std::string topic1 = fmt::format("{::}", color.tag()) + localization->FormatString(LSTR_FMT_RENT_ROOM_FOR_D_GOLD, pPriceRoom);
    int pTopic1Height = pFontArrus->CalcTextHeight(topic1, dialog_window.uFrameWidth, 0);

    color = pDialogueWindow->pCurrentPosActiveItem == 3 ? colorTable.PaleCanary : colorTable.White;
    int foodNum = buildingTable[wData.val - 1].fPriceMultiplier;
    std::string topic2 = fmt::format("{::}", color.tag()) + localization->FormatString(LSTR_FMT_BUY_D_FOOD_FOR_D_GOLD, foodNum, pPriceFood);
    int pTopic2Height = pFontArrus->CalcTextHeight(topic2, dialog_window.uFrameWidth, 0);

    color = pDialogueWindow->pCurrentPosActiveItem == 4 ? colorTable.PaleCanary : colorTable.White;
    std::string topic3 = fmt::format("{::}", color.tag()) + localization->GetString(LSTR_LEARN_SKILLS);
    int pTopic3Height = pFontArrus->CalcTextHeight(topic3, dialog_window.uFrameWidth, 0);

    int pTopic4Height = 0;
    std::string topic4;
    if (houseId() != HOUSE_TAVERN_EMERALD_ISLE) {
        color = pDialogueWindow->pCurrentPosActiveItem == 5 ? colorTable.PaleCanary : colorTable.White;
        topic4 = fmt::format("{::}", color.tag()) + localization->GetString(LSTR_PLAY_ARCOMAGE);
        pTopic4Height = pFontArrus->CalcTextHeight(topic4, dialog_window.uFrameWidth, 0);
    }

    for (int pItemNum = pDialogueWindow->pStartingPosActiveItem; pItemNum < pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton; ++pItemNum) {
        GUIButton *pButton = pDialogueWindow->GetControl(pItemNum);
        if (pButton->msg_param == 15) {
            pButton->uHeight = pTopic1Height;
            pButton->uY = 146;
            pButton->uW = pTopic1Height + 145 + 6;
        } else if (pButton->msg_param == 16) {
            pButton->uHeight = pTopic2Height;
            pButton->uY = pTopic1Height + (pFontArrus->GetHeight() ) + 146;
            pButton->uW = (pTopic1Height + (pFontArrus->GetHeight() ) + 146) + pTopic2Height - 1 + 6;
        } else if (pButton->msg_param == 96) {
            pButton->uY = pTopic1Height + pTopic2Height + 2 * (pFontArrus->GetHeight() ) + 146;
            pButton->uHeight = pTopic3Height;
            pButton->uW = pTopic3Height + (pTopic1Height + pTopic2Height + 2 * (pFontArrus->GetHeight() ) + 146) - 1 + 6;
        } else if (pButton->msg_param == 101) {
            pButton->uHeight = pTopic4Height;
            pButton->uY = pTopic1Height + 3 * (pFontArrus->GetHeight() ) + pTopic4Height + pTopic2Height + 146;
            pButton->uW = (pTopic1Height + 3 * (pFontArrus->GetHeight() ) + pTopic4Height + pTopic2Height + 146) + pTopic4Height - 1 + 6;
        }
    }
    dialog_window.DrawTitleText(pFontArrus, 0, 146, Color(), fmt::format("{}\n \n{}\n \n{}\n \n{}", topic1, topic2, topic3, topic4), 3);
}

void GUIWindow_Tavern::arcomageMainDialogue() {
    GUIWindow dialog_window = *this;
    dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    if (checkIfPlayerCanInteract()) {
        int pOptionsCount = 2;
        pShopOptions[0] = localization->GetString(LSTR_RULES);
        pShopOptions[1] = localization->GetString(LSTR_VICTORY_CONDITIONS);
        if (pParty->hasItem(ITEM_QUEST_ARCOMAGE_DECK)) {
            pShopOptions[2] = localization->GetString(LSTR_PLAY);
            pOptionsCount = 3;
        }
        int all_text_height = 0;
        for (int i = 0; i < pOptionsCount; ++i) {
            all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], dialog_window.uFrameWidth, 0);
        }
        all_text_height = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - all_text_height) / pOptionsCount;

        int vertPos = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pOptionsCount * all_text_height - all_text_height) / 2 - all_text_height / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
        int pNumString = 0;
        for (int pItemNum = pDialogueWindow->pStartingPosActiveItem; pItemNum < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; pItemNum++) {
            GUIButton *pButton = pDialogueWindow->GetControl(pItemNum);
            pButton->uY = all_text_height + vertPos;
            int pTextHeight = pFontArrus->CalcTextHeight(pShopOptions[pNumString], dialog_window.uFrameWidth, 0);
            pButton->uHeight = pTextHeight;
            vertPos = pButton->uY + pTextHeight - 1;
            pButton->uW = vertPos + 6;
            Color pColorText = colorTable.PaleCanary;
            if (pDialogueWindow->pCurrentPosActiveItem != pItemNum) {
                pColorText = colorTable.White;
            }
            dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY, pColorText, pShopOptions[pNumString], 3);
            ++pNumString;
        }
    }
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
    int pTextHeight = pFontArrus->CalcTextHeight(str, dialog_window.uFrameWidth, 12) + 7;
    if (352 - pTextHeight < 8) {
        font = pFontCreate;
        pTextHeight = pFontCreate->CalcTextHeight(str, dialog_window.uFrameWidth, 12) + 7;
    }
    render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
    render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
    DrawText(font, {12, 354 - pTextHeight}, Color(), font->FitTextInAWindow(str, dialog_window.uFrameWidth, 12), 0, 0, Color());
}

void GUIWindow_Tavern::arcomageVictoryCondDialogue() {
    GUIWindow dialog_window = *this;
    dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    std::string label = pNPCTopics[wData.val + 247].pText;
    dialog_window.uFrameWidth = game_viewport_width;
    dialog_window.uFrameZ = 452;
    int pTextHeight = pFontArrus->CalcTextHeight(label, dialog_window.uFrameWidth, 12) + 7;
    render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
    render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
    DrawText(pFontArrus, {12, 354 - pTextHeight}, Color(), pFontArrus->FitTextInAWindow(label, dialog_window.uFrameWidth, 12));
}

void GUIWindow_Tavern::arcomageResultDialogue() {
    GUIWindow dialog_window = *this;
    dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    if (pArcomageGame->bGameInProgress == 1)
        return;
    std::string pText;
    if (pArcomageGame->uGameWinner) {
        if (pArcomageGame->uGameWinner == 1)
            pText = localization->GetString(LSTR_YOU_WON);
        else
            pText = localization->GetString(LSTR_YOU_LOST);
    } else {
        pText = localization->GetString(LSTR_A_TIE);
    }
    int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pFontArrus->CalcTextHeight(pText, dialog_window.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
    dialog_window.DrawTitleText(pFontArrus, 0, vertMargin, colorTable.PaleCanary, pText, 3);
}

void GUIWindow_Tavern::restDialogue() {
    int pPriceRoom = PriceCalculator::tavernRoomCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);

    if (pParty->GetGold() >= pPriceRoom) {
        pParty->TakeGold(pPriceRoom);
        playHouseSound(houseId(), HOUSE_SOUND_TAVERN_RENT_ROOM);
        dialog_menu_id = DIALOGUE_NULL;
        houseDialogPressEscape();
        playHouseGoodbyeSpeech();
        pMediaPlayer->Unload();

        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_RentRoom, wData.val, 1);
        Release();
        window_SpeakInHouse = 0;
        return;
    }
    GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
    playHouseSound(houseId(), HOUSE_SOUND_TAVERN_NOT_ENOUGH_GOLD);
    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
}

void GUIWindow_Tavern::buyFoodDialogue() {
    int pPriceFood = PriceCalculator::tavernFoodCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);

    if ((double)pParty->GetFood() >= buildingTable[wData.val - 1].fPriceMultiplier) {
        GameUI_SetStatusBar(LSTR_RATIONS_FULL);
        if (pParty->hasActiveCharacter()) {
            pParty->activeCharacter().playReaction(SPEECH_PacksFull);
        }
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        return;
    }
    if (pParty->GetGold() >= pPriceFood) {
        pParty->TakeGold(pPriceFood);
        pParty->SetFood(buildingTable[wData.val - 1].fPriceMultiplier);
        playHouseSound(houseId(), HOUSE_SOUND_TAVERN_BUY_FOOD);
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        return;
    }
    GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
    playHouseSound(houseId(), HOUSE_SOUND_TAVERN_NOT_ENOUGH_GOLD);
    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
}

void GUIWindow_Tavern::houseDialogueOptionSelected(DIALOGUE_TYPE option) {
    if (option == DIALOGUE_TAVERN_ARCOMAGE_RESULT) {
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_PlayArcomage, 0, 0);
    } else if (IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
    }
}

void GUIWindow_Tavern::houseSpecificDialogue() {
    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter()) {  // avoid nzi
        pParty->setActiveToFirstCanAct();
    }

    switch (dialog_menu_id) {
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
        learnSkillsDialogue();
        break;
      default:
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }
}

std::vector<DIALOGUE_TYPE> GUIWindow_Tavern::listDialogueOptions(DIALOGUE_TYPE option) {
    switch (option) {
      case DIALOGUE_MAIN:
        if (houseId() == HOUSE_TAVERN_EMERALD_ISLE) {
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

DIALOGUE_TYPE GUIWindow_Tavern::getOptionOnEscape() {
    if (IsSkillLearningDialogue(dialog_menu_id)) {
        return DIALOGUE_LEARN_SKILLS;
    }
    if (dialog_menu_id == DIALOGUE_TAVERN_ARCOMAGE_RULES ||
        dialog_menu_id == DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS ||
        dialog_menu_id == DIALOGUE_TAVERN_ARCOMAGE_RESULT) {
        return DIALOGUE_TAVERN_ARCOMAGE_MAIN;
    }
    if (dialog_menu_id == DIALOGUE_MAIN) {
        return DIALOGUE_NULL;
    }
    return DIALOGUE_MAIN;
}
