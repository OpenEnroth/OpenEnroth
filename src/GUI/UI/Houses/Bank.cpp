#include "GUI/UI/Houses/Bank.h"

#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"

#include "Engine/Localization.h"

void BankDialog() {
    GUIWindow bank_window = *window_SpeakInHouse;
    bank_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    bank_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    bank_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
    bank_window.DrawTitleText(
        pFontArrus, 0, 220, colorTable.PaleCanary,
        fmt::format("{}: {}", localization->GetString(LSTR_BALANCE), pParty->uNumGoldInBank),
        3);
    switch (dialog_menu_id) {
    case DIALOGUE_MAIN:
    {
        Color pColorText = colorTable.PaleCanary;
        if (pDialogueWindow->pCurrentPosActiveItem != 2) {
            pColorText = colorTable.White;
        }
        bank_window.DrawTitleText(pFontArrus, 0, 146, pColorText,
            localization->GetString(LSTR_DEPOSIT), 3);
        pColorText = colorTable.PaleCanary;
        if (pDialogueWindow->pCurrentPosActiveItem != 3) {
            pColorText = colorTable.White;
        }
        bank_window.DrawTitleText(pFontArrus, 0, 176, pColorText,
            localization->GetString(LSTR_WITHDRAW), 3);
        break;
    }
    case DIALOGUE_BANK_PUT_GOLD:
    {
        if (window_SpeakInHouse->keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
            bank_window.DrawTitleText(pFontArrus, 0, 146, colorTable.PaleCanary,
                                      fmt::format("{}\n{}", localization->GetString(LSTR_DEPOSIT), localization->GetString(LSTR_HOW_MUCH)), 3);
            bank_window.DrawTitleText(pFontArrus, 0, 186, colorTable.White, keyboardInputHandler->GetTextInput().c_str(), 3);
            bank_window.DrawFlashingInputCursor(pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput().c_str()) / 2 + 80, 185, pFontArrus);
            return;
        }
        if (window_SpeakInHouse->keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
            int sum = atoi(keyboardInputHandler->GetTextInput().c_str());
            if (sum <= 0) {
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }

            int party_gold = pParty->GetGold();
            if (sum > party_gold) {
                PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_NotEnoughMoney);
                sum = party_gold;
            }

            if (sum > 0) {
                pParty->TakeGold(sum);
                pParty->AddBankGold(sum);
                if (pParty->hasActiveCharacter()) {
                    pParty->activeCharacter().playReaction(SPEECH_BankDeposit);
                }
            }
        }
        window_SpeakInHouse->keyboard_input_status = WINDOW_INPUT_NONE;
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }
    case DIALOGUE_BANK_GET_GOLD:
    {
        if (window_SpeakInHouse->keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
            bank_window.DrawTitleText(pFontArrus, 0, 146, colorTable.PaleCanary,
                                      fmt::format("{}\n{}", localization->GetString(LSTR_WITHDRAW), localization->GetString(LSTR_HOW_MUCH)), 3);
            bank_window.DrawTitleText(pFontArrus, 0, 186, colorTable.White, keyboardInputHandler->GetTextInput().c_str(), 3);
            bank_window.DrawFlashingInputCursor(pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput().c_str()) / 2 + 80, 185, pFontArrus);
            return;
        } else if (window_SpeakInHouse->keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
            window_SpeakInHouse->keyboard_input_status = WINDOW_INPUT_NONE;
            int sum = atoi(keyboardInputHandler->GetTextInput().c_str());
            if (sum <= 0) {
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }

            int bank_gold = pParty->GetBankGold();
            if (sum > bank_gold) {
                PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_NotEnoughMoney);
                sum = bank_gold;
            }

            if (sum > 0) {
                pParty->TakeBankGold(sum);
                pParty->AddGold(sum);
            }
        }
        window_SpeakInHouse->keyboard_input_status = WINDOW_INPUT_NONE;
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }
    default:
        break;
    }
}
