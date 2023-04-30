#pragma once

#include <string>

void eventProcessor(int eventId, int targetObj, bool canShowMessages, int startStep = 0);
bool npcDialogueEventProcessor(int eventId, int startStep = 0);
std::string getEventHintString(int eventId);

void onMapLoad();
void onMapLeave();
void onTimer();
