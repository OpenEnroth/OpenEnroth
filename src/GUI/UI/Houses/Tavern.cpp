#include <string>

#include "GUI/UI/Houses/Tavern.h"

#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIShops.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"

#include "Engine/Localization.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/IRender.h"

#include "Arcomage/Arcomage.h"

#include "Media/MediaPlayer.h"

//----- (004B8285) --------------------------------------------------------
void TavernDialog() {
    int pItemNum;
    double v2;                // st7@1
    int pNumString;           // edi@16
    GUIButton *pButton;       // eax@65
    int pSkillCount;
    signed int pOptionsCount;       // edi@77
    signed int i;                   // esi@79
                                    //  signed int v53; // edi@81
    const char *pText;              // [sp-4h] [bp-278h]@93
    unsigned int pTopic1Height;     // [sp+26Fh] [bp-5h]@55
    uint8_t pTopic2Height;  // [sp+267h] [bp-Dh]@57
    uint8_t pTopic3Height;  // [sp+253h] [bp-21h]@59
    uint8_t pTopic4Height = 0;
    int pTextHeight;
    int all_text_height = 0;  // [sp+260h] [bp-14h]@18
    GUIFont *pOutString;

    GUIWindow dialog_window = *window_SpeakInHouse;
    dialog_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialog_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialog_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter())  // avoid nzi
        pParty->setActiveToFirstCanAct();

    const BuildingDesc &house = buildingTable[window_SpeakInHouse->wData.val - 1];

    int pPriceRoom = PriceCalculator::tavernRoomCostForPlayer(&pParty->activeCharacter(), house);
    int pPriceFood = PriceCalculator::tavernFoodCostForPlayer(&pParty->activeCharacter(), house);

    switch (dialog_menu_id) {
    case DIALOGUE_MAIN:
    {
        if (!HouseUI_CheckIfPlayerCanInteract()) return;

        std::string topic1 = fmt::format("\f{:05}",
                pDialogueWindow->pCurrentPosActiveItem == 2
                ? colorTable.PaleCanary.c16()
                : colorTable.White.c16()) +
                             localization->FormatString(LSTR_FMT_RENT_ROOM_FOR_D_GOLD, pPriceRoom);
        pTopic1Height = pFontArrus->CalcTextHeight(
            topic1, dialog_window.uFrameWidth, 0);

        std::string topic2 = fmt::format("\f{:05}",
                pDialogueWindow->pCurrentPosActiveItem == 3
                ? colorTable.PaleCanary.c16()
                : colorTable.White.c16())
            + localization->FormatString(
                LSTR_FMT_BUY_D_FOOD_FOR_D_GOLD,
                (unsigned int)
                buildingTable[window_SpeakInHouse->wData.val - 1].fPriceMultiplier,
                pPriceFood);
        pTopic2Height = pFontArrus->CalcTextHeight(topic2, dialog_window.uFrameWidth, 0);

        std::string topic3 = fmt::format("\f{:05}",
                pDialogueWindow->pCurrentPosActiveItem == 4
                ? colorTable.PaleCanary.c16()
                : colorTable.White.c16()) +
                             localization->GetString(LSTR_LEARN_SKILLS);
        pTopic3Height = pFontArrus->CalcTextHeight(topic3, dialog_window.uFrameWidth, 0);

        std::string topic4;
        if (window_SpeakInHouse->wData.val >= 108 &&
            window_SpeakInHouse->wData.val <= 120) {
            topic4 =
                fmt::format("\f{:05}",
                    pDialogueWindow->pCurrentPosActiveItem == 5
                    ? colorTable.PaleCanary.c16()
                    : colorTable.White.c16()) +
                localization->GetString(LSTR_PLAY_ARCOMAGE);
            pTopic4Height = pFontArrus->CalcTextHeight(
                topic4, dialog_window.uFrameWidth, 0);
        }

        if (pDialogueWindow->pNumPresenceButton) {
            for (pItemNum = pDialogueWindow->pStartingPosActiveItem;
                pItemNum < pDialogueWindow->pStartingPosActiveItem +
                pDialogueWindow->pNumPresenceButton;
                ++pItemNum) {
                pButton = pDialogueWindow->GetControl(pItemNum);
                if (pButton->msg_param == 15) {
                    pButton->uHeight = pTopic1Height;
                    pButton->uY = 146;
                    pButton->uW = pTopic1Height + 145 + 6;
                } else if (pButton->msg_param == 16) {
                    pButton->uHeight = pTopic2Height;
                    pButton->uY =
                        pTopic1Height + (pFontArrus->GetHeight() ) + 146;
                    pButton->uW = (pTopic1Height +
                        (pFontArrus->GetHeight() ) + 146) +
                        pTopic2Height - 1 + 6;
                } else if (pButton->msg_param == 96) {
                    pButton->uY = pTopic1Height + pTopic2Height +
                        2 * (pFontArrus->GetHeight() ) + 146;
                    pButton->uHeight = pTopic3Height;
                    pButton->uW =
                        pTopic3Height +
                        (pTopic1Height + pTopic2Height +
                            2 * (pFontArrus->GetHeight() ) + 146) -
                        1 + 6;
                } else if (pButton->msg_param == 101) {
                    pButton->uHeight = pTopic4Height;
                    pButton->uY = pTopic1Height +
                        3 * (pFontArrus->GetHeight() ) +
                        pTopic4Height + pTopic2Height + 146;
                    pButton->uW =
                        (pTopic1Height + 3 * (pFontArrus->GetHeight() ) +
                            pTopic4Height + pTopic2Height + 146) +
                        pTopic4Height - 1 + 6;
                }
            }
            dialog_window.DrawTitleText(
                pFontArrus, 0, 146, Color(),
                fmt::format("{}\n \n{}\n \n{}\n \n{}", topic1, topic2, topic3, topic4),
                3);
        }
        break;
    }
    case DIALOGUE_TAVERN_ARCOMAGE_RULES:
    {
        pOutString = pFontArrus;

        std::string str = pNPCTopics[354].pText;
        dialog_window.uFrameWidth = game_viewport_width;
        dialog_window.uFrameZ = 452;
        pTextHeight = pFontArrus->CalcTextHeight(str, dialog_window.uFrameWidth, 12) + 7;
        if (352 - pTextHeight < 8) {
            pOutString = pFontCreate;
            pTextHeight = pFontCreate->CalcTextHeight(str, dialog_window.uFrameWidth, 12) + 7;
        }
        render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
        render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        window_SpeakInHouse->DrawText(pOutString, {12, 354 - pTextHeight}, Color(),
            pOutString->FitTextInAWindow(str, dialog_window.uFrameWidth, 12), 0, 0, Color());
        break;
    }

    case DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS:
    {
        std::string label =
            pNPCTopics[window_SpeakInHouse->wData.val + 247].pText;
        dialog_window.uFrameWidth = game_viewport_width;
        dialog_window.uFrameZ = 452;
        pTextHeight = pFontArrus->CalcTextHeight(
            label, dialog_window.uFrameWidth, 12) +
            7;
        render->DrawTextureCustomHeight(8 / 640.0f,
            (352 - pTextHeight) / 480.0f,
            ui_leather_mm7, pTextHeight);
        render->DrawTextureNew(
            8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        window_SpeakInHouse->DrawText(pFontArrus, {12, 354 - pTextHeight}, Color(),
            pFontArrus->FitTextInAWindow(label, dialog_window.uFrameWidth, 12));
        break;
    }

    case DIALOGUE_TAVERN_ARCOMAGE_RESULT:
    {
        if (pArcomageGame->bGameInProgress == 1) return;
        if (pArcomageGame->uGameWinner) {
            if (pArcomageGame->uGameWinner == 1)
                pText = localization->GetString(LSTR_YOU_WON);
            else
                pText = localization->GetString(LSTR_YOU_LOST);
        } else {
            pText = localization->GetString(LSTR_A_TIE);
        }
        dialog_window.DrawTitleText(
            pFontArrus, 0,
            (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pFontArrus->CalcTextHeight(
                pText, dialog_window.uFrameWidth, 0)) /
            2 +
            SIDE_TEXT_BOX_BODY_TEXT_OFFSET, colorTable.PaleCanary, pText, 3);
        break;
    }
    case DIALOGUE_TAVERN_REST:
    {
        if (pParty->GetGold() >= pPriceRoom) {
            pParty->TakeGold(pPriceRoom);
            PlayHouseSound(window_SpeakInHouse->wData.val,
                HouseSound_NotEnoughMoney);
            dialog_menu_id = DIALOGUE_NULL;
            HouseDialogPressCloseBtn();
            GetHouseGoodbyeSpeech();
            pMediaPlayer->Unload();

            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_RentRoom, window_SpeakInHouse->wData.val, 1);
            window_SpeakInHouse->Release();
            window_SpeakInHouse = 0;
            return;
        }
        GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
        PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Goodbye);
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }

    case DIALOGUE_LEARN_SKILLS:
    {
        if (!HouseUI_CheckIfPlayerCanInteract()) return;
        pSkillCount = 0;
        int pPriceSkill = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), house);
        all_text_height = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
            i < pDialogueWindow->pStartingPosActiveItem +
            pDialogueWindow->pNumPresenceButton;
            ++i) {
            auto skill = GetLearningDialogueSkill(
                (DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param
            );
            if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE
                && !pParty->activeCharacter().pActiveSkills[skill]) {
                all_text_height = pFontArrus->CalcTextHeight(
                    localization->GetSkillName(skill),
                    dialog_window.uFrameWidth, 0);
                pSkillCount++;
            }
        }
        SkillTrainingDialogue(&dialog_window, pSkillCount, all_text_height, pPriceSkill);
        return;
    }

    case DIALOGUE_TAVERN_BUY_FOOD:
    {
        if ((double)pParty->GetFood() >=
            buildingTable[window_SpeakInHouse->wData.val - 1].fPriceMultiplier) {
            GameUI_SetStatusBar(LSTR_RATIONS_FULL);
            if (pParty->hasActiveCharacter())
                pParty->activeCharacter().playReaction(SPEECH_PacksFull);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        if (pParty->GetGold() >= pPriceFood) {
            pParty->TakeGold(pPriceFood);
            pParty->SetFood(buildingTable[window_SpeakInHouse->wData.val - 1].fPriceMultiplier);
            PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Greeting_2);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
        PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Goodbye);
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }

    case DIALOGUE_TAVERN_ARCOMAGE_MAIN:
    {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            pOptionsCount = 2;
            pShopOptions[0] = localization->GetString(LSTR_RULES);
            pShopOptions[1] = localization->GetString(LSTR_VICTORY_CONDITIONS);
            if (pParty->hasItem(ITEM_QUEST_ARCOMAGE_DECK)) {
                pShopOptions[2] = localization->GetString(LSTR_PLAY);
                pOptionsCount = 3;
            }
            for (i = 0; i < pOptionsCount; ++i)
                all_text_height += pFontArrus->CalcTextHeight(pShopOptions[i], dialog_window.uFrameWidth, 0);
            all_text_height = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - all_text_height) / pOptionsCount;

            int v54 = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pOptionsCount * all_text_height - all_text_height) /
                2 - all_text_height / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
            pNumString = 0;
            for (pItemNum = pDialogueWindow->pStartingPosActiveItem;
                pItemNum < pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                pItemNum++) {
                pButton = pDialogueWindow->GetControl(pItemNum);
                pButton->uY = all_text_height + v54;
                pTextHeight = pFontArrus->CalcTextHeight(
                    pShopOptions[pNumString], dialog_window.uFrameWidth, 0);
                pButton->uHeight = pTextHeight;
                v54 = pButton->uY + pTextHeight - 1;
                pButton->uW = v54 + 6;
                Color pColorText = colorTable.PaleCanary;
                if (pDialogueWindow->pCurrentPosActiveItem != pItemNum)
                    pColorText = colorTable.White;
                dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY,
                    pColorText,
                    pShopOptions[pNumString], 3);
                ++pNumString;
            }
        }
        break;
    }
    default: { break; }
    }
}
