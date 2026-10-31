#pragma once

#include <string>

#include "Engine/Evt/EvtContext.h"

/**
 * @offset 0x4613C4
 */
void initDecorationEvents();

/**
 * @offset 0x46CC4B
 */
void checkDecorationEvents();

/**
 * @param context                       Event to run, and what it runs for.
 * @param startStep                     Step to start from.
 */
void runEvent(EvtContext context, int startStep = 0);

/**
 * @param eventId                       Global event behind an NPC's scripted topic.
 * @return                              Whether the topic is listed in the NPC's dialogue.
 */
bool canShowTopic(int eventId);
bool hasEventHint(int eventId);
std::string getEventHintString(int eventId);

void onMapLoad();
void onMapLeave();
void onTimer();
