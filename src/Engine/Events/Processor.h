#pragma once

#include <string>

void eventProcessor(int eventId, int targetObj, bool canShowMessages, int startStep);
std::string getEventHintString(int eventId);

void registerEventTriggers();
void onMapLoad();
void onMapLeave();
