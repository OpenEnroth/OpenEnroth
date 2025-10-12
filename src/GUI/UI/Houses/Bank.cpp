#include "Bank.h"

#include <string>
#include <vector>

#include "GUI/GUIMessageQueue.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"

#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"

#include "Io/KeyboardActionMapping.h"

using Io::TextInputType;

void GUIWindow_Bank::mainDialogue() {
    GUIWindow bank_window = *this;
    bank_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    bank_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    bank_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    std::vector<std::string> optionsText = {localization->str(LSTR_DEPOSIT), localization->str(LSTR_WITHDRAW)};
    std::string balance_str = fmt::format("{}: {}", localization->str(LSTR_BALANCE), pParty->uNumGoldInBank);
    bank_window.DrawTitleText(assets->pFontArrus.get(), 0, 220, colorTable.PaleCanary, balance_str, 3);

    drawOptions(optionsText, colorTable.PaleCanary, 146, true);
}

void GUIWindow_Bank::putGoldDialogue() {
    GUIWindow bank_window = *this;
    bank_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    bank_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    bank_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
    bank_window.DrawTitleText(assets->pFontArrus.get(), 0, 220, colorTable.PaleCanary,
                              fmt::format("{}: {}", localization->str(LSTR_BALANCE), pParty->uNumGoldInBank), 3);

    if (keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
        bank_window.DrawTitleText(assets->pFontArrus.get(), 0, 146, colorTable.PaleCanary,
                                  fmt::format("{}\n{}", localization->str(LSTR_DEPOSIT), localization->str(LSTR_HOW_MUCH)), 3);
        bank_window.DrawTitleText(assets->pFontArrus.get(), 0, 186, colorTable.White, keyboardInputHandler->GetTextInput(), 3);
        bank_window.DrawFlashingInputCursor(assets->pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput()) / 2 + 80, 185, assets->pFontArrus.get());
        return;
    }
    if (keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
        int sum = atoi(keyboardInputHandler->GetTextInput().c_str());
        if (sum <= 0) {
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            return;
        }

        int party_gold = pParty->GetGold();
        if (sum > party_gold) {
            playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
            sum = party_gold;
        }

        if (sum > 0) {
            pParty->TakeGold(sum);
            pParty->AddBankGold(sum);
            _transactionPerformed = true;
            if (pParty->hasActiveCharacter()) {
                pParty->activeCharacter().playReaction(SPEECH_BANK_DEPOSIT);
            }
        }
    }
    keyboard_input_status = WINDOW_INPUT_NONE;
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_Bank::getGoldDialogue() {
    GUIWindow bank_window = *this;
    bank_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    bank_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    bank_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
    bank_window.DrawTitleText(assets->pFontArrus.get(), 0, 220, colorTable.PaleCanary,
                              fmt::format("{}: {}", localization->str(LSTR_BALANCE), pParty->uNumGoldInBank), 3);

    if (keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
        bank_window.DrawTitleText(assets->pFontArrus.get(), 0, 146, colorTable.PaleCanary,
                                  fmt::format("{}\n{}", localization->str(LSTR_WITHDRAW), localization->str(LSTR_HOW_MUCH)), 3);
        bank_window.DrawTitleText(assets->pFontArrus.get(), 0, 186, colorTable.White, keyboardInputHandler->GetTextInput(), 3);
        bank_window.DrawFlashingInputCursor(assets->pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput()) / 2 + 80, 185, assets->pFontArrus.get());
        return;
    } else if (keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
        keyboard_input_status = WINDOW_INPUT_NONE;
        int sum = atoi(keyboardInputHandler->GetTextInput().c_str());
        if (sum <= 0) {
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            return;
        }

        int bank_gold = pParty->GetBankGold();
        if (sum > bank_gold) {
            playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
            sum = bank_gold;
        }

        if (sum > 0) {
            _transactionPerformed = true;
            pParty->TakeBankGold(sum);
            pParty->AddGold(sum);
        }
    }
    keyboard_input_status = WINDOW_INPUT_NONE;
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_Bank::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
    if (option == DIALOGUE_BANK_PUT_GOLD || option == DIALOGUE_BANK_GET_GOLD) {
        keyboardInputHandler->StartTextInput(TextInputType::Number, 10, this);
    }
}

void GUIWindow_Bank::houseSpecificDialogue() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_BANK_PUT_GOLD:
        putGoldDialogue();
        break;
      case DIALOGUE_BANK_GET_GOLD:
        getGoldDialogue();
        break;
      default:
        break;
    }
}

std::vector<DialogueId> GUIWindow_Bank::listDialogueOptions() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        return {DIALOGUE_BANK_PUT_GOLD, DIALOGUE_BANK_GET_GOLD};
      default:
        return {};
    }
}

void GUIWindow_Bank::playHouseGoodbyeSpeech() {
    if (_transactionPerformed) {
        playHouseSound(houseId(), HOUSE_SOUND_BANK_GOODBYE);
    }
}
