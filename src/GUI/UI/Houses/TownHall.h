#pragma once

#include <string>

/**
 * Handler for the "Bounty Hunt" dialogue option in a town hall.
 *
 * Regenerates bounty if needed, gives gold for a completed bounty hunt, and updates the current reply message to
 * be retrieved later with a call to `bountyHuntingText`.
 */
void bountyHuntingDialogueOptionClicked();

/**
 * @return                              Text to show after the player has clicked on the "Bounty Hunt" dialogue option.
 */
std::string bountyHuntingText();

void TownHallDialog();
