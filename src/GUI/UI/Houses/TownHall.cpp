#include <cassert>

#include "GUI/UI/Houses/TownHall.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Localization.h"
#include "Engine/Tables/AwardTable.h"

#include "GUI/UI/UIHouses.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"

static const char *bountyHunting_text = nullptr;                // word_F8B1A4
static int16_t bountyHunting_monsterId = 0;

//----- (004B7911) --------------------------------------------------------
void TownHallDialog() {
    int v1;                       // eax@10
    int v2;                       // esi@10
    signed int pStringSum;        // ebx@24
    signed int v16;               // ebx@28
    int v17;                      // ebx@28
    GUIButton *pButton;           // eax@30
    int pTextHeight;              // eax@30
    Color pTextColor;  // ax@30
    int v29;                      // [sp+10Ch] [bp-10h]@28
    int v31;                      // [sp+114h] [bp-8h]@29
    GUIFont *pOutString;          // [sp+118h] [bp-4h]@21

    GUIWindow townHall_window = *window_SpeakInHouse;
    townHall_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    townHall_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    townHall_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    std::string fine_str = fmt::format("{}: {}", localization->GetString(LSTR_CURRENT_FINE), pParty->uFine);
    townHall_window.DrawTitleText(pFontArrus, 0, 260, colorTable.PaleCanary, fine_str, 3);

    switch (dialog_menu_id) {
    case DIALOGUE_MAIN:
    {
        pStringSum = 1;
        pTextHeight = 0;
        pShopOptions[0] = localization->GetString(LSTR_BOUNTY_HUNT);
        if (pParty->uFine > 0) {
            pShopOptions[1] = localization->GetString(LSTR_PAY_FINE);
            pStringSum = 2;
        }
        for (uint i = 0; i < pStringSum; ++i)
            pTextHeight += pFontArrus->CalcTextHeight(
                pShopOptions[i], townHall_window.uFrameWidth, 0);
        v29 = (100 - pTextHeight) / pStringSum;
        v16 = 80 - pStringSum * ((100 - pTextHeight) / pStringSum);
        v17 = (v16 / 2) - v29 / 2 + 158;
        if (pDialogueWindow->pNumPresenceButton > 0) {
            v31 = 2;
            uint j = 0;
            for (uint i = pDialogueWindow->pStartingPosActiveItem;
                i < pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                ++i) {
                pButton = pDialogueWindow->GetControl(i);
                pButton->uY = v29 + v17;
                pTextHeight = pFontArrus->CalcTextHeight(
                    pShopOptions[j], townHall_window.uFrameWidth, 0);
                pButton->uHeight = pTextHeight;
                v17 = pButton->uY + pTextHeight - 1;
                pButton->uW = v17 + 6;
                pTextColor = colorTable.PaleCanary;
                if (pDialogueWindow->pCurrentPosActiveItem != v31)
                    pTextColor = colorTable.White;
                townHall_window.DrawTitleText(pFontArrus, 0, pButton->uY, pTextColor, pShopOptions[j], 3);
                ++v31;
                ++j;
            }
        }
        break;
    }
    case DIALOGUE_TOWNHALL_MESSAGE:
    {
        current_npc_text = bountyHuntingText();
        GUIWindow window = *pDialogueWindow;
        window.uFrameWidth = 458;
        window.uFrameZ = 457;
        pOutString = pFontArrus;
        pTextHeight = pFontArrus->CalcTextHeight(current_npc_text, window.uFrameWidth, 13) + 7;
        if (352 - pTextHeight < 8) {
            pOutString = pFontCreate;
            pTextHeight = pFontCreate->CalcTextHeight(current_npc_text, window.uFrameWidth, 13) + 7;
        }
        render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
        render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        // window.DrawText(pOutString, 13, 354 - pTextHeight, 0, pOutString->FitTextInAWindow(current_npc_text, window.uFrameWidth, 13), 0, 0, 0);
        window.DrawText(pOutString, {13, 354 - pTextHeight}, Color(), current_npc_text, 0, 0, Color());
        break;
    }
    case DIALOGUE_TOWNHALL_PAY_FINE:
    {
        if (window_SpeakInHouse->keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
            townHall_window.DrawTitleText(pFontArrus, 0, 146, colorTable.PaleCanary,
                                          fmt::format("{}\n{}", localization->GetString(LSTR_PAY), localization->GetString(LSTR_HOW_MUCH)), 3);
            townHall_window.DrawTitleText(pFontArrus, 0, 186, colorTable.White, keyboardInputHandler->GetTextInput(), 3);
            townHall_window.DrawFlashingInputCursor(pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput()) / 2 + 80, 185, pFontArrus);
            return;
        } else if (window_SpeakInHouse->keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
            int sum = atoi(keyboardInputHandler->GetTextInput().c_str());
            if (sum > 0) {
                int party_gold = pParty->GetGold();
                if (sum > party_gold) {
                    PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_NotEnoughMoney);
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
        window_SpeakInHouse->keyboard_input_status = WINDOW_INPUT_NONE;
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }
    default:
        break;
    }
    return;
}

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

void bountyHuntingDialogueOptionClicked() {
    HOUSE_ID house = window_SpeakInHouse->houseId();

    // Generate new bounty
    if (pParty->PartyTimes.bountyHuntNextGenTime[house] < pParty->GetPlayingTime()) {
        pParty->monster_for_hunting_killed[house] = false;
        pParty->PartyTimes.bountyHuntNextGenTime[house] =
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
                        fmt::format("{::}{}{::}", colorTable.PaleCanary.tag(),
                                    pMonsterStats->pInfos[bountyHunting_monsterId].pName, colorTable.White.tag()),
                        100 * pMonsterStats->pInfos[bountyHunting_monsterId].uLevel);
}
