#pragma once

#include <string>
#include <vector>

#include "Engine/Tables/NPCTable.h"
#include "GUI/GUIDialogues.h"

/**
 * @return                              Whether an MM8 dialogue topic id is in retail's reserved 601-649 "Roster Join Event"
 *                                      band, i.e. means "recruit roster.txt row (id - 600)".
 */
bool isRosterJoinTopic(unsigned int eventId);

std::string npcDialogueOptionString(DialogueId topic, NPCData *npcData);

std::vector<DialogueId> prepareScriptedNPCDialogueTopics(NPCData *npcData);

DialogueId handleScriptedNPCTopicSelection(DialogueId topic, NPCData *npcData);
std::vector<DialogueId> listNPCDialogueOptions(DialogueId topic);

void selectSpecialNPCTopicSelection(DialogueId topic, NPCData* npcData);

extern int gold_transaction_amount;
