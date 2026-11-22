#pragma once

#include <optional>

#include "Engine/Pid.h"

void ItemInteraction(int item_id);
bool CanInteractWithActor(int id);
void InteractWithActor(int id);

/**
 * Runs a decoration's event, if it has one.
 *
 * @param id                            Decoration index in `pLevelDecorations`.
 * @param pid                           Decoration's pid.
 * @param actingCharacterIndex          Character the event acts on, or empty for the party's active character.
 */
void DecorationInteraction(int id, Pid pid, std::optional<int> actingCharacterIndex = std::nullopt);

void DoInteractionWithTopmostZObject(Pid pid);
