#include "GUIBountyHunting.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Localization.h"
#include "Engine/Awards.h"

#include "GUI/UI/UIHouses.h"

#include "GUIWindow.h"
#include "GUIButton.h"

const char *bountyHunting_text = nullptr;                // word_F8B1A4

int RandomMonsterForHunting(HOUSE_ID townhall) {
    while (true) {
        int result = grng->Random(258) + 1;
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
            
        case HOUSE_INVALID:
            // TODO(captainurist): this one is called from CheckBountyRespawnAndAward and uses a different table, but for what purpose?
            if ((result < 0x73u || result > 0x84u) &&
                (result < 0x85u || result > 0x96u) &&
                (result < 0xEBu || result > 0xFCu) &&
                (result < 0x97u || result > 0xBAu) &&
                (result < 0xC4u || result > 0xC6u))
                return result;
            break;

        default:
            assert(false);
            return -1;
        }
    }
}

void CheckBountyRespawnAndAward() {
    uDialogueType = DIALOGUE_83_bounty_hunting;
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetRenderDimensions().w, 350}, 0);
    pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid,
                                                    localization->GetString(LSTR_CANCEL), { ui_exit_cancel_button_background }
    );
    pDialogueWindow->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_BuyInShop_Identify_Repair, 0, InputAction::Invalid, "");
    pDialogueWindow->CreateButton({480, 160}, {140, 30}, 1, 0, UIMSG_0, DIALOGUE_83_bounty_hunting, InputAction::Invalid, "");
    pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;

    // get new monster for hunting
    if (pParty->PartyTimes.bountyHunting_next_generation_time[window_SpeakInHouse->houseId()] < pParty->GetPlayingTime()) {
        pParty->monster_for_hunting_killed[window_SpeakInHouse->houseId()] = false;
        pParty->PartyTimes.bountyHunting_next_generation_time[window_SpeakInHouse->houseId()] =
            GameTime((int64_t)((double)(0x12750000 * (pParty->uCurrentMonth + 12 * pParty->uCurrentYear - 14015)) * 0.033333335));
        pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] = RandomMonsterForHunting(HOUSE_INVALID);
    }

    bountyHunting_monster_id_for_hunting = pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()];
    if (!pParty->monster_for_hunting_killed[window_SpeakInHouse->houseId()]) {
        bountyHunting_text = pNPCTopics[351].pText;
        if (!pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()])
            bountyHunting_text = pNPCTopics[353].pText;
    } else {  // get prize
        if (pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()]) {
            pParty->PartyFindsGold(100 * pMonsterStats->pInfos[(uint16_t)pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()]].uLevel, 0);
            for (uint i = 0; i < 4; ++i)
                pParty->pPlayers[i].SetVariable(VAR_Award, Award_BountiesCollected);
            pParty->uNumBountiesCollected += 100 * pMonsterStats->pInfos[pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()]].uLevel;
            pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] = 0;
            pParty->monster_for_hunting_killed[window_SpeakInHouse->houseId()] = false;
        }
        bountyHunting_text = pNPCTopics[352].pText;
    }
}

void DiscussBountyInTownhall() {
    if (pParty->PartyTimes.bountyHunting_next_generation_time[window_SpeakInHouse->houseId()] < pParty->GetPlayingTime()) {  // new generation
        pParty->monster_for_hunting_killed[window_SpeakInHouse->houseId()] = false;
        pParty->PartyTimes.bountyHunting_next_generation_time[window_SpeakInHouse->houseId()] = GameTime((int64_t)((double)(309657600 *
                                                                                                                            (pParty->uCurrentMonth + 12ll * pParty->uCurrentYear - 14015)) * 0.033333335));
        pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] = RandomMonsterForHunting(window_SpeakInHouse->houseId());
    }
    bountyHunting_monster_id_for_hunting = pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()];
    if (!pParty->monster_for_hunting_killed[window_SpeakInHouse->houseId()]) {
        bountyHunting_text = pNPCTopics[351].pText;  // "В этом месяцу назначена награда за голову %s..."
        if (!pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()])
            bountyHunting_text = pNPCTopics[353].pText; // "Кое кто уже приходил в этом месяце за наградой"
    } else {
        if (pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] > 0) {  // get prize
            pParty->PartyFindsGold(
                100 * pMonsterStats->pInfos[(uint16_t)pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()]].uLevel, 0);
            for (uint i = 0; i < 4; ++i)
                pParty->pPlayers[i].SetVariable(VAR_Award, Award_BountiesCollected);
            pParty->uNumBountiesCollected += 100 * pMonsterStats->pInfos[pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()]].uLevel;
            pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] = 0;
            pParty->monster_for_hunting_killed[window_SpeakInHouse->houseId()] = false;
        }
        bountyHunting_text = pNPCTopics[352].pText;  //"Поздравляю! Вы успешно..."
    }
}

std::string BountyHuntingText() {
    return stringPrintf(bountyHunting_text,
                        fmt::format("\f{:05}{}\f{:05}", colorTable.PaleCanary.c16(),
                                    pMonsterStats->pInfos[bountyHunting_monster_id_for_hunting].pName, colorTable.White.c16()).c_str(),
                        100 * pMonsterStats->pInfos[bountyHunting_monster_id_for_hunting].uLevel);
}
