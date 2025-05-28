#include "TownHall.h"

#include <cassert>
#include <vector>
#include <string>

#include "Engine/Objects/Monsters.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Localization.h"
#include "Engine/Data/AwardEnums.h"
#include "Engine/Party.h"
#include "Engine/mm7_data.h"
#include "Engine/Engine.h"

#include "GUI/GUIMessageQueue.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIHouses.h"

#include "Io/KeyboardActionMapping.h"

#include "Engine/Random/Random.h"

#include "Engine/AssetsManager.h"

using Io::TextInputType;

void GUIWindow_TownHall::mainDialogue() {
    GUIWindow townHall_window = *this;
    townHall_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    townHall_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    townHall_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    std::vector<std::string> optionsText = {localization->GetString(LSTR_BOUNTY_HUNT)};
    std::string fine_str = fmt::format("{}: {}", localization->GetString(LSTR_CURRENT_FINE), pParty->uFine);
    townHall_window.DrawTitleText(assets->pFontArrus.get(), 0, 260, colorTable.PaleCanary, fine_str, 3);

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
    townHall_window.DrawTitleText(assets->pFontArrus.get(), 0, 260, colorTable.PaleCanary, fine_str, 3);

    current_npc_text = bountyHuntingText();
    GUIWindow window = *pDialogueWindow;
    window.uFrameWidth = 458;
    window.uFrameZ = 457;
    GUIFont *pOutString = assets->pFontArrus.get();
    int pTextHeight = assets->pFontArrus->CalcTextHeight(current_npc_text, window.uFrameWidth, 13) + 7;
    if (352 - pTextHeight < 8) {
        pOutString = assets->pFontCreate.get();
        pTextHeight = assets->pFontCreate->CalcTextHeight(current_npc_text, window.uFrameWidth, 13) + 7;
    }
    render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
    render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
    window.DrawText(pOutString, {13, 354 - pTextHeight}, colorTable.White, current_npc_text);
}

void GUIWindow_TownHall::payFineDialogue() {
    GUIWindow townHall_window = *this;
    townHall_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    townHall_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    townHall_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    std::string fine_str = fmt::format("{}: {}", localization->GetString(LSTR_CURRENT_FINE), pParty->uFine);
    townHall_window.DrawTitleText(assets->pFontArrus.get(), 0, 260, colorTable.PaleCanary, fine_str, 3);

    if (keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
        townHall_window.DrawTitleText(assets->pFontArrus.get(), 0, 146, colorTable.PaleCanary,
                                      fmt::format("{}\n{}", localization->GetString(LSTR_PAY), localization->GetString(LSTR_HOW_MUCH)), 3);
        townHall_window.DrawTitleText(assets->pFontArrus.get(), 0, 186, colorTable.White, keyboardInputHandler->GetTextInput(), 3);
        townHall_window.DrawFlashingInputCursor(assets->pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput()) / 2 + 80, 185, assets->pFontArrus.get());
        return;
    } else if (keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
        int sum = atoi(keyboardInputHandler->GetTextInput().c_str());
        if (sum > 0) {
            int party_gold = pParty->GetGold();
            if (sum > party_gold) {
                // TODO(Nik-RE-dev): game resources does not contain such sounds for town halls
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
                    pParty->activeCharacter().playReaction(SPEECH_BANK_DEPOSIT);
            }
        }
    }
    keyboard_input_status = WINDOW_INPUT_NONE;
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_TownHall::houseSpecificDialogue() {
    switch (_currentDialogue) {
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

void GUIWindow_TownHall::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
    if (option == DIALOGUE_TOWNHALL_BOUNTY_HUNT) {
        bountyHuntingDialogueOptionClicked();
    } else if (option == DIALOGUE_TOWNHALL_PAY_FINE) {
        keyboardInputHandler->StartTextInput(TextInputType::Number, 10, this);
    }
}

std::vector<DialogueId> GUIWindow_TownHall::listDialogueOptions() {
    switch (_currentDialogue) {
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

MonsterId GUIWindow_TownHall::randomMonsterForHunting(HouseId townhall) {
    while (true) {
        MonsterId result = grng->randomSample(allMonsters());
        if (isBountyHuntable(monsterTypeForMonsterId(result), townhall))
            return result;
    }
}

void GUIWindow_TownHall::bountyHuntingDialogueOptionClicked() {
    HouseId house = houseId();

    // Generate new bounty
    if (pParty->PartyTimes.bountyHuntNextGenTime[house] < pParty->GetPlayingTime()) {
        pParty->monster_for_hunting_killed[house] = false;
        pParty->PartyTimes.bountyHuntNextGenTime[house] = Time::fromMonths(pParty->GetPlayingTime().toMonths() + 1);
        pParty->monster_id_for_hunting[house] = randomMonsterForHunting(house);
    }

    _bountyHuntMonsterId = pParty->monster_id_for_hunting[house];

    if (!pParty->monster_for_hunting_killed[house]) {
        if (pParty->monster_id_for_hunting[house] != MONSTER_INVALID) {
            _bountyHuntText = pNPCTopics[351].pText; // "This month's bounty is on a %s..."
        } else {
            _bountyHuntText = pNPCTopics[353].pText; // "Someone has already claimed the bounty this month..."
        }
    } else {
        // Get prize
        if (pParty->monster_id_for_hunting[house] != MONSTER_INVALID) {
            int bounty = 100 * pMonsterStats->infos[pParty->monster_id_for_hunting[house]].level;

            pParty->partyFindsGold(bounty, GOLD_RECEIVE_SHARE);
            for (Character &player : pParty->pCharacters) {
                player.SetVariable(VAR_Award, Award_BountiesCollected);
            }
            pParty->uNumBountiesCollected += bounty;
            pParty->monster_id_for_hunting[house] = MONSTER_INVALID;
            pParty->monster_for_hunting_killed[house] = false;
        }

        _bountyHuntText = pNPCTopics[352].pText; // "Congratulations on defeating the %s! Here is the %lu gold reward..."
    }
}

std::string GUIWindow_TownHall::bountyHuntingText() {
    assert(!_bountyHuntText.empty());

    // This happens when you claim a bounty and revisit the town hall the same month.
    // Assumes _bountyHuntText is already containing the "someone has already" text (pNPCTopics[353]).
    if (_bountyHuntMonsterId == MONSTER_INVALID)
        return _bountyHuntText;

    // TODO(captainurist): what do we do with exceptions inside fmt?
    std::string name = fmt::format("{::}{}{::}", colorTable.PaleCanary.tag(), pMonsterStats->infos[_bountyHuntMonsterId].name, colorTable.White.tag());
    return fmt::sprintf(_bountyHuntText, name, 100 * pMonsterStats->infos[_bountyHuntMonsterId].level); // NOLINT: this is not ::sprintf.
}
