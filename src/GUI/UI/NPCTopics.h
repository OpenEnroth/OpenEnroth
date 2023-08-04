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

std::string npcDialogueOptionString(DIALOGUE_TYPE topic, NPCData *npcData);

std::vector<DIALOGUE_TYPE> prepareScriptedNPCDialogueTopics(NPCData *npcData);

std::vector<DIALOGUE_TYPE> handleScriptedNPCTopicSelection(DIALOGUE_TYPE topic, NPCData *npcData);

void selectSpecialNPCTopicSelection(DIALOGUE_TYPE topic, NPCData* npcData);

extern int gold_transaction_amount;
