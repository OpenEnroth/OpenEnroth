#pragma once

#include <array>
#include <string>
#include <vector>

#include "Utility/Geometry/Vec.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Objects/CharacterEnums.h"
#include "GUI/GUIDialogues.h"
#include "GUI/UI/UIHouseEnums.h"

void Arena_SelectionFightLevel();

/**
 * @offset 0x4BC109
 */
void ArenaFight();

/**
 * @offset 0x4B254D
 */
std::string masteryTeacherOptionString();

/**
 * @offset 0x4B29F2
 */
const std::string &joinGuildOptionString();

/**
 * @offset 0x4B40E6
 */
void NPCHireableDialogPrepare();

std::string npcDialogueOptionString(DIALOGUE_TYPE topic, NPCData *npcData);

std::vector<DIALOGUE_TYPE> prepareScriptedNPCDialogueTopics(NPCData *npcData);

// TODO(Nik-RE-dev): currently this function handles dialogue buttons creation etc,
//                   need to move such functionality into UIHouses/UIDialogue
std::vector<DIALOGUE_TYPE> handleScriptedNPCTopicSelection(DIALOGUE_TYPE topic, NPCData *npcData);
void selectSpecialNPCTopicSelection(DIALOGUE_TYPE topic, NPCData* npcData);

extern int gold_transaction_amount;
