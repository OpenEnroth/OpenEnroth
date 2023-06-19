#include <cassert>

#include "GUI/UI/Houses/TownHall.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Localization.h"
#include "Engine/Tables/AwardTable.h"

#include "GUI/UI/UIHouses.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"

#include "Io/KeyboardActionMapping.h"

using Io::TextInputType;

void GUIWindow_TownHall::mainDialogue() {
    GUIWindow townHall_window = *this;
    townHall_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    townHall_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    townHall_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    std::vector<std::string> optionsText = {localization->GetString(LSTR_BOUNTY_HUNT)};
    std::string fine_str = fmt::format("{}: {}", localization->GetString(LSTR_CURRENT_FINE), pParty->uFine);
    townHall_window.DrawTitleText(pFontArrus, 0, 260, colorTable.PaleCanary, fine_str, 3);

    if (pParty->uFine > 0) {
        optionsText.push_back(localization->GetString(LSTR_PAY_FINE));
    }

    drawOptions(optionsText, colorTable.PaleCanary, 170, true);
}

void GUIWindow_TownHall::bountyHuntDialogue() {
    GUIWindow townHall_window = *this;
    townHall_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    townHall_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    townHall_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    std::string fine_str = fmt::format("{}: {}", localization->GetString(LSTR_CURRENT_FINE), pParty->uFine);
    townHall_window.DrawTitleText(pFontArrus, 0, 260, colorTable.PaleCanary, fine_str, 3);

    current_npc_text = bountyHuntingText();
    GUIWindow window = *pDialogueWindow;
    window.uFrameWidth = 458;
    window.uFrameZ = 457;
    GUIFont *pOutString = pFontArrus;
    int pTextHeight = pFontArrus->CalcTextHeight(current_npc_text, window.uFrameWidth, 13) + 7;
    if (352 - pTextHeight < 8) {
        pOutString = pFontCreate;
        pTextHeight = pFontCreate->CalcTextHeight(current_npc_text, window.uFrameWidth, 13) + 7;
    }
    render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
    render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
    window.DrawText(pOutString, {13, 354 - pTextHeight}, Color(), current_npc_text);
}

void GUIWindow_TownHall::payFineDialogue() {
    GUIWindow townHall_window = *this;
    townHall_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    townHall_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    townHall_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    std::string fine_str = fmt::format("{}: {}", localization->GetString(LSTR_CURRENT_FINE), pParty->uFine);
    townHall_window.DrawTitleText(pFontArrus, 0, 260, colorTable.PaleCanary, fine_str, 3);

    if (keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
        townHall_window.DrawTitleText(pFontArrus, 0, 146, colorTable.PaleCanary,
                                      fmt::format("{}\n{}", localization->GetString(LSTR_PAY), localization->GetString(LSTR_HOW_MUCH)), 3);
        townHall_window.DrawTitleText(pFontArrus, 0, 186, colorTable.White, keyboardInputHandler->GetTextInput(), 3);
        townHall_window.DrawFlashingInputCursor(pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput()) / 2 + 80, 185, pFontArrus);
        return;
    } else if (keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
        int sum = atoi(keyboardInputHandler->GetTextInput().c_str());
        if (sum > 0) {
            int party_gold = pParty->GetGold();
            if (sum > party_gold) {
                playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
                sum = party_gold;
            }

            if (sum > 0) {
                int required_sum = pParty->GetFine();
                if (sum > required_sum)
                    sum = required_sum;

                pParty->TakeGold(sum);
                pParty->TakeFine(sum);
                if (pParty->hasActiveCharacter())
                    pParty->activeCharacter().playReaction(SPEECH_BankDeposit);
            }
        }
    }
    keyboard_input_status = WINDOW_INPUT_NONE;
    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
}

void GUIWindow_TownHall::houseSpecificDialogue() {
    switch (dialog_menu_id) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_TOWNHALL_BOUNTY_HUNT:
        bountyHuntDialogue();
        break;
      case DIALOGUE_TOWNHALL_PAY_FINE:
        payFineDialogue();
        break;
      default:
        break;
    }
}

void GUIWindow_TownHall::houseDialogueOptionSelected(DIALOGUE_TYPE option) {
    if (option == DIALOGUE_TOWNHALL_BOUNTY_HUNT) {
        bountyHuntingDialogueOptionClicked();
    } else if (option == DIALOGUE_TOWNHALL_PAY_FINE) {
        keyboardInputHandler->StartTextInput(TextInputType::Number, 10, this);
    }
}

std::vector<DIALOGUE_TYPE> GUIWindow_TownHall::listDialogueOptions(DIALOGUE_TYPE option) {
    switch (option) {
      case DIALOGUE_MAIN:
        if (pParty->uFine) {
            return {DIALOGUE_TOWNHALL_BOUNTY_HUNT, DIALOGUE_TOWNHALL_PAY_FINE};
        } else {
            return {DIALOGUE_TOWNHALL_BOUNTY_HUNT};
        }
      default:
        return {};
    }
}

int GUIWindow_TownHall::randomMonsterForHunting(HOUSE_ID townhall) {
    while (true) {
        int result = grng->random(258) + 1;
        switch (townhall) {
        case HOUSE_TOWNHALL_HARMONDALE:
            if ((result < 115 || result > 132) &&
                (result < 235 || result > 252) &&
                (result < 133 || result > 150) &&
                (result < 0x97u || result > 0xBAu) &&
                (result < 0xBEu || result > 0xC0u) &&
                (result < 0xC4u || result > 0xC6u) &&
                (result < 0x2Bu || result > 0x2Du) &&
                (result < 0xCDu || result > 0xCFu) &&
                (result < 0x5Eu || result > 0x60u) &&
                (result < 0xFDu || result > 0xFFu) &&
                (result < 0x6Du || result > 0x6Fu) &&
                (result < 0x61u || result > 0x63u))
                return result;
            break;

        case HOUSE_TOWNHALL_ERATHIA:
            if ((result < 115 || result > 132) &&
                (result < 0xE8u || result > 0xF9u) &&
                (result < 0x85u || result > 0x96u) &&
                (result < 0x97u || result > 0xBAu) &&
                (result < 0xBEu || result > 0xC0u) &&
                (result < 0xC4u || result > 0xC6u) &&
                (result < 0x2Bu || result > 0x2Du) &&
                (result < 0x52u || result > 0x54u) &&
                (result < 4 || result > 6) &&
                (result < 0x37u || result > 0x39u) &&
                (result < 0x3Au || result > 0x3Cu) &&
                (result < 0x3Du || result > 0x3Fu) &&
                (result < 0xFDu || result > 0xFFu) &&
                (result < 0x61u || result > 0x63u) &&
                (result < 0xCDu || result > 0xCFu))
                return result;
            break;

        case HOUSE_TOWNHALL_TULAREAN_FOREST:
            if ((result < 0x73u || result > 0x84u) &&
                (result < 0xE8u || result > 0xF9u) &&
                (result < 0x85u || result > 0x96u) &&
                (result < 0x97u || result > 0xBAu) &&
                (result < 0xBEu || result > 0xC0u) &&
                (result < 0xC4u || result > 0xC6u) &&
                (result < 0x2Bu || result > 0x2Du) &&
                (result < 0x31u || result > 0x33u) &&
                (result < 0x34u || result > 0x36u) &&
                (result < 0xFDu || result > 0xFFu) &&
                (result < 0x61u || result > 0x63u) &&
                (result < 0x1Cu || result > 0x1Eu))
                return result;
            break;

        case HOUSE_TOWNHALL_CELESTE:
            if ((result < 0x73u || result > 0x84u) &&
                (result < 0xE8u || result > 0xF9u) &&
                (result < 0x85u || result > 0x96u) &&
                (result < 0x97u || result > 0xBAu) &&
                (result < 0xBEu || result > 0xC0u) &&
                (result < 0xC4u || result > 0xC6u) &&
                (result < 0x2Bu || result > 0x2Du) &&
                (result < 0x5Eu || result > 0x60u) &&
                (result < 0x43u || result > 0x45u) &&
                (result < 0x4Fu || result > 0x51u) &&
                (result < 0xC1u || result > 0xC3u) &&
                (result < 0x13u || result > 0x15u) &&
                (result < 0xFDu || result > 0xFFu) &&
                (result < 0x61u || result > 0x63u) &&
                (result < 0x6Au || result > 0x6Cu))
                return result;
            break;

        case HOUSE_TOWNHALL_THE_PIT:
            if ((result < 0x73u || result > 0x84u) &&
                (result < 0xE8u || result > 0xF9u) &&
                (result < 0x85u || result > 0x96u) &&
                (result < 0x97u || result > 0xBAu) &&
                (result < 0xBEu || result > 0xC0u) &&
                (result < 0xC4u || result > 0xC6u) &&
                (result < 0x2Bu || result > 0x2Du) &&
                (result < 0x6Du || result > 0x6Fu) &&
                (result < 0x46u || result > 0x48u) &&
                (result < 0x100u || result > 0x102u) &&
                (result < 0xD9u || result > 0xDBu) &&
                (result < 0xC7u || result > 0xC9u) &&
                (result < 0xE5u || result > 0xE7u) &&
                (result < 0xDFu || result > 0xE1u) &&
                (result < 0x5Bu || result > 0x5Du) &&
                (result < 0x49u || result > 0x4Bu) &&
                (result < 0xFDu || result > 0xFFu) &&
                (result < 0x61u || result > 0x63u) &&
                (result < 0x10u || result > 0x12u))
                return result;
            break;

        default:
            assert(false);
            return -1;
        }
    }
}

void GUIWindow_TownHall::bountyHuntingDialogueOptionClicked() {
    HOUSE_ID house = houseId();

    // Generate new bounty
    if (pParty->PartyTimes.bountyHuntNextGenTime[house] < pParty->GetPlayingTime()) {
        pParty->monster_for_hunting_killed[house] = false;
        pParty->PartyTimes.bountyHuntNextGenTime[house] = GameTime::FromMonths(pParty->GetPlayingTime().GetMonths() + 1);
        pParty->monster_id_for_hunting[house] = randomMonsterForHunting(house);
    }

    _bountyHuntMonsterId = pParty->monster_id_for_hunting[house];

    if (!pParty->monster_for_hunting_killed[house]) {
        if (pParty->monster_id_for_hunting[house]) {
            _bountyHuntText = pNPCTopics[351].pText; // "This month's bounty is on a %s..."
        } else {
            _bountyHuntText = pNPCTopics[353].pText; // "Someone has already claimed the bounty this month..."
        }
    } else {
        // Get prize
        if (pParty->monster_id_for_hunting[house]) {
            int bounty = 100 * pMonsterStats->pInfos[pParty->monster_id_for_hunting[house]].uLevel;

            pParty->partyFindsGold(bounty, GOLD_RECEIVE_SHARE);
            for (Player &player : pParty->pPlayers) {
                player.SetVariable(VAR_Award, Award_BountiesCollected);
            }
            pParty->uNumBountiesCollected += bounty;
            pParty->monster_id_for_hunting[house] = 0;
            pParty->monster_for_hunting_killed[house] = false;
        }

        _bountyHuntText = pNPCTopics[352].pText; // "Congratulations on defeating the %s! Here is the %lu gold reward..."
    }
}

std::string GUIWindow_TownHall::bountyHuntingText() {
    assert(!_bountyHuntText.empty());
    assert(_bountyHuntMonsterId != 0);

    // TODO(captainurist): what do we do with exceptions inside fmt?
    std::string name = fmt::format("{::}{}{::}", colorTable.PaleCanary.tag(), pMonsterStats->pInfos[_bountyHuntMonsterId].pName, colorTable.White.tag());
    return fmt::sprintf(_bountyHuntText, name, 100 * pMonsterStats->pInfos[_bountyHuntMonsterId].uLevel);
}
