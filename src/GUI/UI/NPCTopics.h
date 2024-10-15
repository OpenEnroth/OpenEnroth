#pragma once

#include <string>
#include <vector>

#include "Engine/Tables/NPCTable.h"
#include "GUI/GUIDialogues.h"

std::string npcDialogueOptionString(DialogueId topic, NPCData *npcData);

std::vector<DialogueId> prepareScriptedNPCDialogueTopics(NPCData *npcData);

DialogueId handleScriptedNPCTopicSelection(DialogueId topic, NPCData *npcData);
std::vector<DialogueId> listNPCDialogueOptions(DialogueId topic);

void selectSpecialNPCTopicSelection(DialogueId topic, NPCData* npcData);

extern int gold_transaction_amount;
