#pragma once

#include <stdint.h>
#include <array>
#include <string>
#include <vector>
#include "Utility/Geometry/Vec.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Objects/CharacterEnums.h"
#include "GUI/GUIDialogues.h"
#include "GUI/UI/UIHouseEnums.h"

enum DIALOGUE_TYPE : int32_t;
struct NPCData;

std::string npcDialogueOptionString(DIALOGUE_TYPE topic, NPCData *npcData);

std::vector<DIALOGUE_TYPE> prepareScriptedNPCDialogueTopics(NPCData *npcData);

DIALOGUE_TYPE handleScriptedNPCTopicSelection(DIALOGUE_TYPE topic, NPCData *npcData);
std::vector<DIALOGUE_TYPE> listNPCDialogueOptions(DIALOGUE_TYPE topic);

void selectSpecialNPCTopicSelection(DIALOGUE_TYPE topic, NPCData* npcData);

extern int gold_transaction_amount;
