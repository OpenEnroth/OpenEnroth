#include "GUIBountyHunting.h"

#include <cassert>

#include "Engine/Graphics/IRender.h"
#include "Engine/Localization.h"
#include "Engine/Awards.h"

#include "GUI/UI/UIHouses.h"

#include "GUIWindow.h"
#include "GUIButton.h"

static const char *bountyHunting_text = nullptr;                // word_F8B1A4
static int16_t bountyHunting_monsterId = 0;

static int RandomMonsterForHunting(HOUSE_ID townhall) {
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

void openBountyHuntingDialogue() {
    uDialogueType = DIALOGUE_83_bounty_hunting;
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetRenderDimensions().w, 350}, 0);
    pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid,
                                                    localization->GetString(LSTR_CANCEL), { ui_exit_cancel_button_background });
    pDialogueWindow->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_BuyInShop_Identify_Repair, 0, InputAction::Invalid, "");
    pDialogueWindow->CreateButton({480, 160}, {140, 30}, 1, 0, UIMSG_0, DIALOGUE_83_bounty_hunting, InputAction::Invalid, "");
    pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;

    bountyHuntingDialogueOptionClicked();
}

void bountyHuntingDialogueOptionClicked() {
    HOUSE_ID house = window_SpeakInHouse->houseId();

    // Generate new bounty
    if (pParty->PartyTimes.bountyHunting_next_generation_time[house] < pParty->GetPlayingTime()) {
        pParty->monster_for_hunting_killed[house] = false;
        pParty->PartyTimes.bountyHunting_next_generation_time[house] =
            GameTime(0x12750000ll * (pParty->uCurrentMonth + 12ll * pParty->uCurrentYear - 14015ll) / 30ll);
        pParty->monster_id_for_hunting[house] = RandomMonsterForHunting(house);
    }

    bountyHunting_monsterId = pParty->monster_id_for_hunting[house];

    if (!pParty->monster_for_hunting_killed[house]) {
        if (pParty->monster_id_for_hunting[house]) {
            bountyHunting_text = pNPCTopics[351].pText; // "This month's bounty is on a %s..."
        } else {
            bountyHunting_text = pNPCTopics[353].pText; // "Someone has already claimed the bounty this month..."
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

        bountyHunting_text = pNPCTopics[352].pText; // "Congratulations on defeating the %s! Here is the %lu gold reward..."
    }
}

std::string bountyHuntingText() {
    assert(bountyHunting_text);
    assert(bountyHunting_monsterId != 0);

    // TODO(captainurist): what do we do with exceptions inside fmt?
    return fmt::sprintf(bountyHunting_text,
                        fmt::format("\f{:05}{}\f{:05}", colorTable.PaleCanary.c16(),
                                    pMonsterStats->pInfos[bountyHunting_monsterId].pName, colorTable.White.c16()).c_str(),
                        100 * pMonsterStats->pInfos[bountyHunting_monsterId].uLevel);
}
