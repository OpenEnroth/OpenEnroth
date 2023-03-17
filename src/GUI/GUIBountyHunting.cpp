#include "GUIBountyHunting.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Localization.h"
#include "Engine/Awards.h"

#include "GUI/UI/UIHouses.h"

#include "GUIWindow.h"
#include "GUIButton.h"

const char *bountyHunting_text = nullptr;                // word_F8B1A4

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

void DiscussBountyInTownhall() {
    int v16;
    int16_t v24;

    if (pParty->PartyTimes.bountyHunting_next_generation_time[window_SpeakInHouse->houseId()] < pParty->GetPlayingTime()) {  // new generation
        pParty->monster_for_hunting_killed[window_SpeakInHouse->houseId()] = false;
        pParty->PartyTimes.bountyHunting_next_generation_time[window_SpeakInHouse->houseId()] = GameTime((int64_t)((double)(309657600 *
                                                                                                                            (pParty->uCurrentMonth + 12ll * pParty->uCurrentYear - 14015)) * 0.033333335));
        pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] = grng->Random(258) + 1;
        v16 = window_SpeakInHouse->wData.val - HOUSE_TOWNHALL_HARMONDALE;
        if (!v16) {
            while (1) {
                v24 = pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()];
                if ((uint16_t)v24 < 115 ||
                    (uint16_t)v24 > 132) {
                    if (((uint16_t)v24 < 235 ||
                         (uint16_t)v24 > 252) &&
                        ((uint16_t)v24 < 133 ||
                         (uint16_t)v24 > 150) &&
                        ((uint16_t)v24 < 0x97u ||
                         (uint16_t)v24 > 0xBAu) &&
                        ((uint16_t)v24 < 0xBEu ||
                         (uint16_t)v24 > 0xC0u) &&
                        ((uint16_t)v24 < 0xC4u ||
                         (uint16_t)v24 > 0xC6u) &&
                        ((uint16_t)v24 < 0x2Bu ||
                         (uint16_t)v24 > 0x2Du) &&
                        ((uint16_t)v24 < 0xCDu ||
                         (uint16_t)v24 > 0xCFu) &&
                        ((uint16_t)v24 < 0x5Eu ||
                         (uint16_t)v24 > 0x60u) &&
                        ((uint16_t)v24 < 0xFDu ||
                         (uint16_t)v24 > 0xFFu) &&
                        ((uint16_t)v24 < 0x6Du ||
                         (uint16_t)v24 > 0x6Fu) &&
                        ((uint16_t)v24 < 0x61u ||
                         (uint16_t)v24 > 0x63u))
                        break;
                }
                pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] = grng->Random(258) + 1;
            }
        }
        if (v16 == 1) {
            while (1) {
                v24 = pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()];
                if ((uint16_t)v24 < 115 ||
                    (uint16_t)v24 > 132) {
                    if (((uint16_t)v24 < 0xE8u ||
                         (uint16_t)v24 > 0xF9u) &&
                        ((uint16_t)v24 < 0x85u ||
                         (uint16_t)v24 > 0x96u) &&
                        ((uint16_t)v24 < 0x97u ||
                         (uint16_t)v24 > 0xBAu) &&
                        ((uint16_t)v24 < 0xBEu ||
                         (uint16_t)v24 > 0xC0u) &&
                        ((uint16_t)v24 < 0xC4u ||
                         (uint16_t)v24 > 0xC6u) &&
                        ((uint16_t)v24 < 0x2Bu ||
                         (uint16_t)v24 > 0x2Du) &&
                        ((uint16_t)v24 < 0x52u ||
                         (uint16_t)v24 > 0x54u) &&
                        ((uint16_t)v24 < 4 ||
                         (uint16_t)v24 > 6) &&
                        ((uint16_t)v24 < 0x37u ||
                         (uint16_t)v24 > 0x39u) &&
                        ((uint16_t)v24 < 0x3Au ||
                         (uint16_t)v24 > 0x3Cu) &&
                        ((uint16_t)v24 < 0x3Du ||
                         (uint16_t)v24 > 0x3Fu) &&
                        ((uint16_t)v24 < 0xFDu ||
                         (uint16_t)v24 > 0xFFu) &&
                        ((uint16_t)v24 < 0x61u ||
                         (uint16_t)v24 > 0x63u) &&
                        ((uint16_t)v24 < 0xCDu ||
                         (uint16_t)v24 > 0xCFu))
                        break;
                }
                pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] = grng->Random(258) + 1;
            }
        }
        if (v16 == 2) {
            while (1) {
                v24 = pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()];
                if ((uint16_t)v24 < 0x73u ||
                    (uint16_t)v24 > 0x84u) {
                    if (((uint16_t)v24 < 0xE8u ||
                         (uint16_t)v24 > 0xF9u) &&
                        ((uint16_t)v24 < 0x85u ||
                         (uint16_t)v24 > 0x96u) &&
                        ((uint16_t)v24 < 0x97u ||
                         (uint16_t)v24 > 0xBAu) &&
                        ((uint16_t)v24 < 0xBEu ||
                         (uint16_t)v24 > 0xC0u) &&
                        ((uint16_t)v24 < 0xC4u ||
                         (uint16_t)v24 > 0xC6u) &&
                        ((uint16_t)v24 < 0x2Bu ||
                         (uint16_t)v24 > 0x2Du) &&
                        ((uint16_t)v24 < 0x31u ||
                         (uint16_t)v24 > 0x33u) &&
                        ((uint16_t)v24 < 0x34u ||
                         (uint16_t)v24 > 0x36u) &&
                        ((uint16_t)v24 < 0xFDu ||
                         (uint16_t)v24 > 0xFFu) &&
                        ((uint16_t)v24 < 0x61u ||
                         (uint16_t)v24 > 0x63u) &&
                        ((uint16_t)v24 < 0x1Cu ||
                         (uint16_t)v24 > 0x1Eu))
                        break;
                }
                pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] = grng->Random(258) + 1;
            }
        }
        if (v16 == 3) {
            while (1) {
                v24 = pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()];
                if ((uint16_t)v24 < 0x73u ||
                    (uint16_t)v24 > 0x84u) {
                    if (((uint16_t)v24 < 0xE8u ||
                         (uint16_t)v24 > 0xF9u) &&
                        ((uint16_t)v24 < 0x85u ||
                         (uint16_t)v24 > 0x96u) &&
                        ((uint16_t)v24 < 0x97u ||
                         (uint16_t)v24 > 0xBAu) &&
                        ((uint16_t)v24 < 0xBEu ||
                         (uint16_t)v24 > 0xC0u) &&
                        ((uint16_t)v24 < 0xC4u ||
                         (uint16_t)v24 > 0xC6u) &&
                        ((uint16_t)v24 < 0x2Bu ||
                         (uint16_t)v24 > 0x2Du) &&
                        ((uint16_t)v24 < 0x5Eu ||
                         (uint16_t)v24 > 0x60u) &&
                        ((uint16_t)v24 < 0x43u ||
                         (uint16_t)v24 > 0x45u) &&
                        ((uint16_t)v24 < 0x4Fu ||
                         (uint16_t)v24 > 0x51u) &&
                        ((uint16_t)v24 < 0xC1u ||
                         (uint16_t)v24 > 0xC3u) &&
                        ((uint16_t)v24 < 0x13u ||
                         (uint16_t)v24 > 0x15u) &&
                        ((uint16_t)v24 < 0xFDu ||
                         (uint16_t)v24 > 0xFFu) &&
                        ((uint16_t)v24 < 0x61u ||
                         (uint16_t)v24 > 0x63u) &&
                        ((uint16_t)v24 < 0x6Au ||
                         (uint16_t)v24 > 0x6Cu))
                        break;
                }
                pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] = grng->Random(258) + 1;
            }
        }
        if (v16 == 4) {
            while (1) {
                v24 = pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()];
                if ((uint16_t)v24 < 0x73u ||
                    (uint16_t)v24 > 0x84u) {
                    if (((uint16_t)v24 < 0xE8u ||
                         (uint16_t)v24 > 0xF9u) &&
                        ((uint16_t)v24 < 0x85u ||
                         (uint16_t)v24 > 0x96u) &&
                        ((uint16_t)v24 < 0x97u ||
                         (uint16_t)v24 > 0xBAu) &&
                        ((uint16_t)v24 < 0xBEu ||
                         (uint16_t)v24 > 0xC0u) &&
                        ((uint16_t)v24 < 0xC4u ||
                         (uint16_t)v24 > 0xC6u) &&
                        ((uint16_t)v24 < 0x2Bu ||
                         (uint16_t)v24 > 0x2Du) &&
                        ((uint16_t)v24 < 0x6Du ||
                         (uint16_t)v24 > 0x6Fu) &&
                        ((uint16_t)v24 < 0x46u ||
                         (uint16_t)v24 > 0x48u) &&
                        ((uint16_t)v24 < 0x100u ||
                         (uint16_t)v24 > 0x102u) &&
                        ((uint16_t)v24 < 0xD9u ||
                         (uint16_t)v24 > 0xDBu) &&
                        ((uint16_t)v24 < 0xC7u ||
                         (uint16_t)v24 > 0xC9u) &&
                        ((uint16_t)v24 < 0xE5u ||
                         (uint16_t)v24 > 0xE7u) &&
                        ((uint16_t)v24 < 0xDFu ||
                         (uint16_t)v24 > 0xE1u) &&
                        ((uint16_t)v24 < 0x5Bu ||
                         (uint16_t)v24 > 0x5Du) &&
                        ((uint16_t)v24 < 0x49u ||
                         (uint16_t)v24 > 0x4Bu) &&
                        ((uint16_t)v24 < 0xFDu ||
                         (uint16_t)v24 > 0xFFu) &&
                        ((uint16_t)v24 < 0x61u ||
                         (uint16_t)v24 > 0x63u) &&
                        ((uint16_t)v24 < 0x10u ||
                         (uint16_t)v24 > 0x12u))
                        break;
                }
                pParty->monster_id_for_hunting[window_SpeakInHouse->houseId()] = grng->Random(258) + 1;
            }
        }
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
