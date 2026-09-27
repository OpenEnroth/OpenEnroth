#pragma once

#include "Engine/Pid.h"

void ItemInteraction(int item_id);
bool CanInteractWithActor(int id);
void InteractWithActor(int id);
void DecorationInteraction(int id, Pid pid);
void DoInteractionWithTopmostZObject(Pid pid);
