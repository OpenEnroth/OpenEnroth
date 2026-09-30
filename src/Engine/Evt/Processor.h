#pragma once

#include <functional>
#include <string>

#include "Engine/Pid.h"

struct LevelDecoration;

/**
 * @offset 0x4613C4
 */
void initDecorationEvents();

/**
 * @offset 0x46CC4B
 */
void checkDecorationEvents();

void eventProcessor(int eventId, Pid targetObj, bool canShowMessages, int startStep = 0);

/**
 * @param continuation                  What goes on with the event that stops for the dialogue it opened, once the
 *                                      dialogue closes and lets it. Empty if nothing waits for the dialogue.
 */
void setEventContinuation(std::function<void()> continuation);

bool hasEventContinuation();

/**
 * Called when a dialogue that an event stopped for closes and lets the event go on.
 */
void continueSavedEvent();

/**
 * Called when a dialogue that an event stopped for closes without letting the event go on.
 */
void cancelSavedEvent();

bool npcDialogueEventProcessor(int eventId, int startStep = 0);
bool hasEventHint(int eventId);
std::string getEventHintString(int eventId);

void onMapLoad();
void onMapLeave();
void onTimer();

extern LevelDecoration *savedDecoration;
