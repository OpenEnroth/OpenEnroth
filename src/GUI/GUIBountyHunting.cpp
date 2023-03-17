#include "GUIBountyHunting.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Localization.h"
#include "Engine/Awards.h"

#include "GUI/UI/UIHouses.h"

#include "GUIWindow.h"
#include "GUIButton.h"

void CheckBountyRespawnAndAward() {
    int i;                // eax@2
    int rand_monster_id;  // edx@3

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
    if (pParty->PartyTimes.bountyHunting_next_generation_time[window_SpeakInHouse->houseId()] <
        pParty->GetPlayingTime()) {
        pParty->monster_for_hunting_killed[window_SpeakInHouse->houseId()] = false;
        pParty->PartyTimes.bountyHunting_next_generation_time[window_SpeakInHouse->houseId()] =
            GameTime((int64_t)((double)(0x12750000 * (pParty->uCurrentMonth + 12 * pParty->uCurrentYear - 14015)) * 0.033333335));
        for (;;) {
            rand_monster_id = grng->Random(258) + 1;
            pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] = rand_monster_id;
            if ((uint16_t)rand_monster_id < 0x73u || (uint16_t)rand_monster_id > 0x84u) {
                if (((uint16_t)rand_monster_id < 0xEBu ||
                     (uint16_t)rand_monster_id > 0xFCu) && ((uint16_t)rand_monster_id < 0x85u ||
                                                            (uint16_t)rand_monster_id > 0x96u) && ((uint16_t)rand_monster_id < 0x97u ||
                                                                                                   (uint16_t)rand_monster_id > 0xBAu) && ((uint16_t)rand_monster_id < 0xC4u ||
                                                                                                                                          (uint16_t)rand_monster_id > 0xC6u))
                    break;
            }
        }
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
