#pragma once

#include <array>
#include <string>
#include <vector>

#include "Utility/Geometry/Vec.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Objects/CharacterEnums.h"
#include "GUI/GUIDialogues.h"
#include "GUI/UI/UIHouseEnums.h"

void Arena_SelectionFightLevel();

/**
 * @offset 0x4BC109
 */
void ArenaFight();

/**
 * @offset 0x004B1ECE.
 *
 * @brief Oracle's 'I lost it!' dialog option
 */
void OracleDialogue();

/**
 * @offset 0x4B254D
 */
std::string masteryTeacherOptionString();

/**
 * @offset 0x4B3EF0
 */
void DrawJoinGuildWindow(GUILD_ID guild_id);

/**
 * @offset 0x4B29F2
 */
const std::string &GetJoinGuildDialogueOption(GUILD_ID guild_id);

/**
 * @offset 0x4B40E6
 */
void NPCHireableDialogPrepare();

// TODO(Nik-RE-dev): currently this function handles dialogue buttons creation etc,
//                   need to move such functionality into UIHouses/UIDialogue
std::vector<DIALOGUE_TYPE> handleScriptedNPCTopicSelection(DIALOGUE_TYPE topic, int eventId);
void selectSpecialNPCTopicSelection(DIALOGUE_TYPE topic);

extern AwardType dword_F8B1AC_award_bit_number;
extern int guild_membership_approved;
extern int gold_transaction_amount;
