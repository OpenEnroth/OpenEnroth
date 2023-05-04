#pragma once

#include <string>

/**
 * @offset 0x4613C4
 */
void initDecorationEvents();

/**
 * @offset 0x46CC4B
 */
void checkDecorationEvents();

void eventProcessor(int eventId, int targetObj, bool canShowMessages, int startStep = 0);
bool npcDialogueEventProcessor(int eventId, int startStep = 0);
bool hasEventHint(int eventId);
std::string getEventHintString(int eventId);

void onMapLoad();
void onMapLeave();
void onTimer();

extern int savedEventID;
extern int savedEventStep;
extern struct LevelDecoration *savedDecoration;
