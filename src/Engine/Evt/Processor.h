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
 * @param continuation                  Callback that resumes an event paused on a dialogue, or `nullptr` for none.
 */
void setEventContinuation(std::function<void()> continuation);

bool hasEventContinuation();

/**
 * Runs the stored continuation, if there is one, and clears it.
 */
void continueSavedEvent();

/**
 * Clears the stored continuation without running it.
 */
void cancelSavedEvent();

bool npcDialogueEventProcessor(int eventId, int startStep = 0);
bool hasEventHint(int eventId);
std::string getEventHintString(int eventId);

void onMapLoad();
void onMapLeave();
void onTimer();

extern LevelDecoration *savedDecoration;
