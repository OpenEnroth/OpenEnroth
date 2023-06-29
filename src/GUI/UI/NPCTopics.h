#pragma once

#include <array>
#include <string>

#include "Utility/Geometry/Vec.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Objects/CharacterEnums.h"
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
std::string _4B254D_SkillMasteryTeacher(int trainerInfo);

/**
 * @offset 0x4B3EF0
 */
void DrawJoinGuildWindow(GUILD_ID guild_id);

/**
 * @offset 0x4B29F2
 */
const std::string &GetJoinGuildDialogueOption(GUILD_ID guild_id);

/**
 * @offset 0x4B3FE5
 */
void _4B3FE5_training_dialogue(int a4);

/**
 * @offset 0x4B40E6
 */
void NPCHireableDialogPrepare();

extern AwardType dword_F8B1AC_award_bit_number;
extern CharacterSkillType dword_F8B1AC_skill_being_taught;
